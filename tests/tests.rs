#[cfg(test)]
mod tests {
    use futures::executor::block_on;
    use wgpu::util::DeviceExt;
    use std::convert::TryInto;
    use itertools::izip;

    #[test]
    fn basic() {
        assert_eq!(4,2+2);
    }
    #[actix_rt::test]
    async fn sscal() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (1..10).map(|v| v as f32).collect();

        let a:f32 = 2.;

        let slice_size = x.len() * std::mem::size_of::<f32>();
        let size = slice_size as wgpu::BufferAddress;

        let staging_buffer = device.create_buffer(&wgpu::BufferDescriptor {
            label: None,
            size,
            usage: wgpu::BufferUsage::MAP_READ | wgpu::BufferUsage::COPY_DST,
            mapped_at_creation: false,
        });
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::COPY_SRC
        });

        let (bind_group,bind_group_layout) = get_compute_bind_group(&device,&[&storage_buffer_x]);

        let shader = device.create_shader_module(wgpu::include_spirv!("../spir-v/sscal.spv"));

        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[std::mem::size_of::<f32>()]);

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });
        unsafe {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.set_push_constants(0,std::mem::transmute(&[a][..]));
            cpass.dispatch(x.len() as u32, 1, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }
        // Sets adds copy operation to command encoder.
        // Will copy data from storage buffer on GPU to staging buffer on CPU.
        encoder.copy_buffer_to_buffer(&storage_buffer_x, 0, &staging_buffer, 0, size);

        queue.submit(Some(encoder.finish()));

        let buffer_slice = staging_buffer.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        //block_on(); // Blocks thread until buffer_future can be read
        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            let result:Vec<f32> = data.chunks_exact(4).map(|b| f32::from_ne_bytes(b.try_into().unwrap())).collect();

            drop(data);
            staging_buffer.unmap();

            println!("{:.?}",result);

            for (x,result) in izip!(x.into_iter(),result.into_iter()) {
                assert_eq!(result,x*a);
            }
        }

        //let cs_module = device.create_shader_module(wgpu::include_spirv!("../spir-v/sum.spv"));
    }
    #[actix_rt::test]
    async fn saxpy() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (1..10).map(|v| v as f32).collect();
        let y:Vec<f32> = (10..19).map(|v| v as f32).collect();

        let a:f32 = 2.;

        let slice_size = x.len() * std::mem::size_of::<f32>();
        let size = slice_size as wgpu::BufferAddress;

        let staging_buffer = device.create_buffer(&wgpu::BufferDescriptor {
            label: None,
            size,
            usage: wgpu::BufferUsage::MAP_READ | wgpu::BufferUsage::COPY_DST,
            mapped_at_creation: false,
        });
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE
        });
        let storage_buffer_y = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer Y"),
            contents: bytemuck::cast_slice(&y),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::COPY_SRC
        });

        let (bind_group,bind_group_layout) = get_compute_bind_group(&device,&[&storage_buffer_x,&storage_buffer_y]);

        let shader = device.create_shader_module(wgpu::include_spirv!("../spir-v/saxpy.spv"));

        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[std::mem::size_of::<f32>()]);

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });
        unsafe {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.set_push_constants(0,std::mem::transmute(&[a][..]));
            cpass.dispatch(x.len() as u32, 1, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }
        // Sets adds copy operation to command encoder.
        // Will copy data from storage buffer on GPU to staging buffer on CPU.
        encoder.copy_buffer_to_buffer(&storage_buffer_y, 0, &staging_buffer, 0, size);

        queue.submit(Some(encoder.finish()));

        let buffer_slice = staging_buffer.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        //block_on(); // Blocks thread until buffer_future can be read
        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            let result:Vec<f32> = data.chunks_exact(4).map(|b| f32::from_ne_bytes(b.try_into().unwrap())).collect();

            drop(data);
            staging_buffer.unmap();

            println!("{:.?}",result);

            for (x,y,result) in izip!(x.into_iter(),y.into_iter(),result.into_iter()) {
                assert_eq!(result,y+x*a);
            }
        }
        //assert!(false);
    }
    #[actix_rt::test]
    async fn sdot() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (1..10).map(|v| v as f32).collect();
        let y:Vec<f32> = (10..19).map(|v| v as f32).collect();

        let a:f32 = 2.;

        let slice_size = x.len() * std::mem::size_of::<f32>();
        let size = slice_size as wgpu::BufferAddress;

        let staging_buffer = device.create_buffer(&wgpu::BufferDescriptor {
            label: None,
            size,
            usage: wgpu::BufferUsage::MAP_READ | wgpu::BufferUsage::COPY_DST,
            mapped_at_creation: false,
        });
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x), // Casts [u32] to [u8]
            usage: wgpu::BufferUsage::STORAGE
        });
        let storage_buffer_y = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer Y"),
            contents: bytemuck::cast_slice(&y),
            usage: wgpu::BufferUsage::STORAGE
        });
        let storage_buffer_outputs = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Storage Buffer Outputs"),
            size, // Casts [u32] to [u8]
            usage: wgpu::BufferUsage::STORAGE
                | wgpu::BufferUsage::COPY_SRC,
            mapped_at_creation: false,
        });

        let (bind_group,bind_group_layout) = get_compute_bind_group(&device,&[&storage_buffer_x,&storage_buffer_y,&storage_buffer_outputs]);

        let shader = device.create_shader_module(wgpu::include_spirv!("../spir-v/sdot.spv"));

        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[]);

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });
        {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.dispatch(x.len() as u32, 1, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }
        // Sets adds copy operation to command encoder.
        // Will copy data from storage buffer on GPU to staging buffer on CPU.
        encoder.copy_buffer_to_buffer(&storage_buffer_outputs, 0, &staging_buffer, 0, size);

        queue.submit(Some(encoder.finish()));

        let buffer_slice = staging_buffer.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        //block_on(); // Blocks thread until buffer_future can be read
        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            
            let number_of_vals = data.len()/1024+4;
            println!("{}->{} ({})",data.len(),number_of_vals,number_of_vals/4);

            let result:Vec<f32> = data[0..number_of_vals].chunks_exact(4).map(|b| f32::from_ne_bytes(b.try_into().unwrap())).collect();
            let final_sum:f32 = result.iter().sum();

            drop(data);
            staging_buffer.unmap();

            println!("{:.?}",result);
            println!("{:.?}",final_sum);

            assert_eq!(final_sum,izip!(x.into_iter(),y.into_iter()).map(|(x,y)| x*y).sum());

            // for (x,y,result) in izip!(x.into_iter(),y.into_iter(),result.into_iter()) {
            //     assert_eq!(result,y+x*a);
            // }
        }
        //assert!(false);
    }
    #[actix_rt::test]
    async fn snrm2() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (1..10).map(|v| v as f32).collect();

        let slice_size = x.len() * std::mem::size_of::<f32>();
        let size = slice_size as wgpu::BufferAddress;

        let staging_buffer = device.create_buffer(&wgpu::BufferDescriptor {
            label: None,
            size,
            usage: wgpu::BufferUsage::MAP_READ | wgpu::BufferUsage::COPY_DST,
            mapped_at_creation: false,
        });
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE
        });
        let storage_buffer_outputs = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Storage Buffer Outputs"),
            size,
            usage: wgpu::BufferUsage::STORAGE
                | wgpu::BufferUsage::COPY_SRC,
            mapped_at_creation: false,
        });

        let (bind_group,bind_group_layout) = get_compute_bind_group(&device,&[&storage_buffer_x,&storage_buffer_outputs]);

        let shader = device.create_shader_module(wgpu::include_spirv!("../spir-v/snrm2.spv"));

        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[]);

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });
        {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.dispatch(x.len() as u32, 1, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }
        // Sets adds copy operation to command encoder.
        // Will copy data from storage buffer on GPU to staging buffer on CPU.
        encoder.copy_buffer_to_buffer(&storage_buffer_outputs, 0, &staging_buffer, 0, size);

        queue.submit(Some(encoder.finish()));

        let buffer_slice = staging_buffer.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        //block_on(); // Blocks thread until buffer_future can be read
        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            
            let number_of_vals = data.len()/1024+4;
            println!("{}->{} ({})",data.len(),number_of_vals,number_of_vals/4);

            let result:Vec<f32> = data[0..number_of_vals].chunks_exact(4).map(|b| f32::from_ne_bytes(b.try_into().unwrap())).collect();
            let final_sum:f32 = result.iter().sum::<f32>().sqrt();

            drop(data);
            staging_buffer.unmap();

            println!("{:.?}",result);
            println!("{:.?}",final_sum);

            let expected = x.iter().map(|x| x*x).sum::<f32>().sqrt();
            assert_eq!(final_sum,expected);

            // for (x,y,result) in izip!(x.into_iter(),y.into_iter(),result.into_iter()) {
            //     assert_eq!(result,y+x*a);
            // }
        }
    }
    #[actix_rt::test]
    async fn sasum() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (-5..5).map(|v| v as f32).collect();

        let slice_size = x.len() * std::mem::size_of::<f32>();
        let size = slice_size as wgpu::BufferAddress;

        let staging_buffer = device.create_buffer(&wgpu::BufferDescriptor {
            label: None,
            size,
            usage: wgpu::BufferUsage::MAP_READ | wgpu::BufferUsage::COPY_DST,
            mapped_at_creation: false,
        });
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE
        });
        let storage_buffer_outputs = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Storage Buffer Outputs"),
            size,
            usage: wgpu::BufferUsage::STORAGE
                | wgpu::BufferUsage::COPY_SRC,
            mapped_at_creation: false,
        });

        let (bind_group,bind_group_layout) = get_compute_bind_group(&device,&[&storage_buffer_x,&storage_buffer_outputs]);

        let shader = device.create_shader_module(wgpu::include_spirv!("../spir-v/sasum.spv"));

        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[]);

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });
        {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.dispatch(x.len() as u32, 1, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }
        // Sets adds copy operation to command encoder.
        // Will copy data from storage buffer on GPU to staging buffer on CPU.
        encoder.copy_buffer_to_buffer(&storage_buffer_outputs, 0, &staging_buffer, 0, size);

        queue.submit(Some(encoder.finish()));

        let buffer_slice = staging_buffer.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        //block_on(); // Blocks thread until buffer_future can be read
        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            
            let number_of_vals = data.len()/1024+4;
            println!("{}->{} ({})",data.len(),number_of_vals,number_of_vals/4);

            let result:Vec<f32> = data[0..number_of_vals].chunks_exact(4).map(|b| f32::from_ne_bytes(b.try_into().unwrap())).collect();
            let final_sum:f32 = result.iter().sum();

            drop(data);
            staging_buffer.unmap();

            println!("{:.?}",result);
            println!("{:.?}",final_sum);

            let expected:f32 = x.iter().map(|x| x.abs()).sum();
            assert_eq!(final_sum,expected);

            // for (x,y,result) in izip!(x.into_iter(),y.into_iter(),result.into_iter()) {
            //     assert_eq!(result,y+x*a);
            // }
        }
        //assert!(false);
    }
    #[actix_rt::test]
    async fn sgemv() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (0..10).map(|v| v as f32).collect();
        let y:Vec<f32> = (10..20).map(|v| v as f32).collect();
        let A:Vec<f32> = (0..100).map(|v| v as f32).collect();

        let alpha:f32 = 2.;
        let beta:f32 = 0.5;

        let slice_size = x.len() * std::mem::size_of::<f32>();
        let size = slice_size as wgpu::BufferAddress;

        let A_slice_size = A.len() * std::mem::size_of::<f32>();
        let A_size = slice_size as wgpu::BufferAddress;

        let staging_buffer = device.create_buffer(&wgpu::BufferDescriptor {
            label: None,
            size,
            usage: wgpu::BufferUsage::MAP_READ | wgpu::BufferUsage::COPY_DST,
            mapped_at_creation: false,
        });
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE
        });
        let storage_buffer_y = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer Y"),
            contents: bytemuck::cast_slice(&y),
            usage: wgpu::BufferUsage::STORAGE
        });
        let storage_buffer_A = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer A"),
            contents: bytemuck::cast_slice(&A),
            usage: wgpu::BufferUsage::STORAGE
        });
        let storage_buffer_outputs = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Storage Buffer Outputs"),
            size,
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::COPY_SRC,
            mapped_at_creation: false,
        });

        let (bind_group,bind_group_layout) = get_compute_bind_group(&device,&[&storage_buffer_x,&storage_buffer_y,&storage_buffer_A,&storage_buffer_outputs]);

        let shader = device.create_shader_module(wgpu::include_spirv!("../spir-v/sgemv.spv"));

        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[4 * std::mem::size_of::<f32>()]);

        let c:u32 = 10;

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });
        unsafe {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.set_push_constants(0,std::mem::transmute(&[alpha,beta][..]));
            cpass.dispatch(10, 10, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }
        // Sets adds copy operation to command encoder.
        // Will copy data from storage buffer on GPU to staging buffer on CPU.
        encoder.copy_buffer_to_buffer(&storage_buffer_outputs, 0, &staging_buffer, 0, size);

        queue.submit(Some(encoder.finish()));

        let buffer_slice = staging_buffer.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        //block_on(); // Blocks thread until buffer_future can be read
        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            let result:Vec<f32> = data.chunks_exact(4).map(|b| f32::from_ne_bytes(b.try_into().unwrap())).collect();

            drop(data);
            staging_buffer.unmap();

            println!("{:.?}",result);

            // for (x,y,result) in izip!(x.into_iter(),y.into_iter(),result.into_iter()) {
            //     assert_eq!(result,y+x*a);
            // }
        }
        assert!(false);
    }
   

    async fn get_compute_device_queue() -> (wgpu::Device,wgpu::Queue) {
        let instance = wgpu::Instance::new(wgpu::BackendBit::PRIMARY);
        let adapter = instance
            .request_adapter(&wgpu::RequestAdapterOptions {
                power_preference: wgpu::PowerPreference::Default,
                compatible_surface: None,
            })
            .await
            .unwrap();
    
        let mut limits = wgpu::Limits::default();
        limits.max_push_constant_size = 128u32;
    
        let (device, queue) = adapter
        .request_device(
            &wgpu::DeviceDescriptor {
                features: wgpu::Features::PUSH_CONSTANTS,
                limits: limits,
                shader_validation: true,
            },
            None,
        )
        .await
        .unwrap();
    
        return (device,queue);
    }
    
    fn get_compute_bind_group(device: &wgpu::Device, storage_buffers:&[&wgpu::Buffer]) -> (wgpu::BindGroup,wgpu::BindGroupLayout) {
        let desc:Vec<wgpu::BindGroupLayoutEntry> = storage_buffers.iter().enumerate().map(|(indx,_)| get_BindGroupLayoutEntry(indx)).collect();
        
        let bind_group_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
            label: None,
            entries: &desc,
        });
    
        let desc:Vec<wgpu::BindGroupEntry> = storage_buffers.iter().enumerate().map(|(indx,buffer)| get_BindGroupEntry(indx,buffer)).collect();
    
        // Instantiates the bind group, once again specifying the binding of buffers.
        let bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: None,
            layout: &bind_group_layout,
            entries: &desc,
        });
    
        return (bind_group,bind_group_layout);
    
        fn get_BindGroupLayoutEntry(binding:usize) -> wgpu::BindGroupLayoutEntry {
            wgpu::BindGroupLayoutEntry {
                binding: binding as u32,                             // The location
                visibility: wgpu::ShaderStage::COMPUTE, // Which shader type in the pipeline this buffer is available to.
                ty: wgpu::BindingType::StorageBuffer {
                    dynamic: false,
                    readonly: false, // Specifies if the buffer can only be read within the shader
                    min_binding_size: None,
                },
                count: None,
            }
        }
        fn get_BindGroupEntry(binding:usize,buffer:&wgpu::Buffer) -> wgpu::BindGroupEntry {
            wgpu::BindGroupEntry {
                binding: binding as u32,
                resource: wgpu::BindingResource::Buffer(buffer.slice(..)),
            }
        }
    }
    
    fn get_compute_pipeline(device: &wgpu::Device, bind_group_layout:wgpu::BindGroupLayout, shader:wgpu::ShaderModule, push_constant_sizes:&[usize]) -> wgpu::ComputePipeline {
        let mut push_constant_ranges:Vec<wgpu::PushConstantRange> = if push_constant_sizes.len() > 0 {
            vec![wgpu::PushConstantRange {stages: wgpu::ShaderStage::COMPUTE, range: std::ops::Range {start:0u32, end: push_constant_sizes[0] as u32 }}]
        } else { Vec::new() };
        for i in 1..push_constant_sizes.len() {
            push_constant_ranges.push(wgpu::PushConstantRange {stages: wgpu::ShaderStage::COMPUTE, range: std::ops::Range {start:push_constant_ranges[i-1].range.start, end: push_constant_sizes[i] as u32 }});
        }
    
        let pipeline_layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
            label: None,
            bind_group_layouts: &[&bind_group_layout],
            push_constant_ranges: push_constant_ranges.as_slice(),
        });
    
        // Instantiates the pipeline.
        let compute_pipeline = device.create_compute_pipeline(&wgpu::ComputePipelineDescriptor {
            label: None,
            layout: Some(&pipeline_layout),
            compute_stage: wgpu::ProgrammableStageDescriptor {
                module: &shader,
                entry_point: "main",
            },
        });
    
        return compute_pipeline;
    }
}
