// ----------------------------------------------------------------
// EXTREME WIP STUFF
// YOU HAVE BEEN WARNED
// ----------------------------------------------------------------

#[cfg(test)]
mod tests {
    use wgpu::util::DeviceExt;
    use std::{convert::TryInto,time::Instant};

    const MATRIX_SIZE: usize = 4;
    const VECTOR_SIZE: usize = MATRIX_SIZE * MATRIX_SIZE; 

    const WORKGROUP_SIZE: usize = 1024;
    
    #[actix_rt::test]
    async fn sscal() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (0..VECTOR_SIZE).map(|v| v as f32).collect();
        println!("x.len(): {}",x.len());

        let a:f32 = 2.;
        println!("a: {}",a);
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_READ | wgpu::BufferUsage::MAP_WRITE
        });

        let (bind_group,bind_group_layout) = get_compute_bind_group(&device,&[&storage_buffer_x]);

        let shader = device.create_shader_module(wgpu::include_spirv!("../glsl/sscal.spv"));

        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[std::mem::size_of::<f32>()]);

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });

        let workgroups: u32 = (x.len() as f32 / WORKGROUP_SIZE as f32).ceil() as u32;

        unsafe {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.set_push_constants(0,std::mem::transmute(&[a][..]));
            cpass.dispatch(workgroups, 1, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }

        let command_buffer = encoder.finish();

        let start = Instant::now();

        queue.submit(Some(command_buffer));

        let buffer_slice = storage_buffer_x.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        //block_on(); // Blocks thread until buffer_future can be read
        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            println!("GPU: {} micros",start.elapsed().as_micros());

            let gpu_vec:Vec<f32> = data.chunks_exact(4).map(|b| f32::from_ne_bytes(b.try_into().unwrap())).collect();

            drop(data);
            storage_buffer_x.unmap();

            let start = Instant::now();
            let cpu_vec:Vec<f32> = x.iter().map(|v| v*a).collect();
            println!("CPU: {} micros",start.elapsed().as_micros());

            println!("gpu_vec.len(): {}",gpu_vec.len());
            println!("cpu_vec.len(): {}",cpu_vec.len());

            for (actual,expected) in gpu_vec.iter().zip(cpu_vec.iter()) {
                assert_eq!(actual,expected);
            }
        }
    }
    #[actix_rt::test]
    async fn saxpy() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (0..VECTOR_SIZE).map(|v| v as f32).collect();
        println!("x.len(): {}",x.len());
        let y:Vec<f32> = (0..VECTOR_SIZE).map(|v| v as f32).collect();
        println!("y.len(): {}",y.len());

        let a:f32 = 2.;
        println!("a: {}",a);

        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_WRITE
        });
        let storage_buffer_y = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer Y"),
            contents: bytemuck::cast_slice(&y),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_WRITE | wgpu::BufferUsage::MAP_READ
        });

        println!("-2");

        let (bind_group,bind_group_layout) = get_compute_bind_group(&device,&[&storage_buffer_x,&storage_buffer_y]);

        let shader = device.create_shader_module(wgpu::include_spirv!("../glsl/saxpy.spv"));

        println!("-1");

        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[std::mem::size_of::<f32>()]);

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });

        println!("-0.5");

        let workgroups: u32 = (x.len() as f32 / WORKGROUP_SIZE as f32).ceil() as u32;

        println!("-0.25");

        unsafe {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.set_push_constants(0,std::mem::transmute(&[a][..]));
            cpass.dispatch(workgroups, 1, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }

        println!("0");

        let command_buffer = encoder.finish();

        let start =  Instant::now();

        queue.submit(Some(command_buffer));

        let buffer_slice = storage_buffer_y.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            println!("GPU: {} micros",start.elapsed().as_micros());

            let gpu_vec:Vec<f32> = data.chunks_exact(4).map(|b| f32::from_ne_bytes(b.try_into().unwrap())).collect();

            drop(data);
            storage_buffer_y.unmap();

            let start = Instant::now();

            let cpu_vec:Vec<f32> = x.into_iter().zip(y.into_iter()).map(|(x,y)| y+x*a).collect();
            println!("CPU: {} micros",start.elapsed().as_micros());

            println!("gpu_vec.len(): {}",gpu_vec.len());
            println!("cpu_vec.len(): {}",cpu_vec.len());

            for (actual,expected) in gpu_vec.iter().zip(cpu_vec.iter()) {
                assert_eq!(actual,expected);
            }
        }
    }
    #[ignore] // Fails due to lack of wgpu support for subgroups and/or floating point atomics
    #[actix_rt::test]
    async fn sdot() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (0..VECTOR_SIZE).map(|_| 3f32).collect();
        println!("x.len(): {}",x.len());
        let y:Vec<f32> = (0..VECTOR_SIZE).map(|_| 2f32).collect();
        println!("y.len(): {}",y.len());

        let output_size = std::mem::size_of::<f32>() as wgpu::BufferAddress;
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_WRITE
        });
        let storage_buffer_y = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer Y"),
            contents: bytemuck::cast_slice(&y),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_WRITE
        });
        let storage_buffer_outputs = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Storage Buffer Outputs"),
            size: output_size,
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_READ,
            mapped_at_creation: false,
        });

        let (bind_group,bind_group_layout) = get_compute_bind_group(&device,&[&storage_buffer_x,&storage_buffer_y,&storage_buffer_outputs]);

        let shader = device.create_shader_module(wgpu::include_spirv!("../glsl/sdot.spv"));

        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[]);

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });

        let workgroups: u32 = (x.len() as f32 / WORKGROUP_SIZE as f32).ceil() as u32;

        {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.dispatch(workgroups, 1, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }

        let command_buffer = encoder.finish();

        let start = Instant::now();

        queue.submit(Some(command_buffer));

        let buffer_slice = storage_buffer_outputs.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            println!("GPU: {} micros",start.elapsed().as_micros());
            
            let gpu_sum = f32::from_ne_bytes(data[0..4].try_into().unwrap());

            drop(data);
            storage_buffer_outputs.unmap();

            let start = Instant::now();

            let cpu_sum:f32 = x.iter().zip(y.iter()).map(|(x,y)| x*y).sum();

            println!("CPU: {} micros",start.elapsed().as_micros());

            assert_eq!(gpu_sum,cpu_sum);
        }
    }
    #[ignore] // Fails due to lack of wgpu support for subgroups and/or floating point atomics
    #[actix_rt::test]
    async fn snrm2() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (0..VECTOR_SIZE).map(|_| 3f32).collect();
    
        let output_size = std::mem::size_of::<f32>() as wgpu::BufferAddress;

        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_WRITE
        });
        let storage_buffer_outputs = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Storage Buffer Outputs"),
            size: output_size,
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_READ,
            mapped_at_creation: false,
        });

        let (bind_group,bind_group_layout) = get_compute_bind_group(&device,&[&storage_buffer_x,&storage_buffer_outputs]);

        let shader = device.create_shader_module(wgpu::include_spirv!("../glsl/snrm2.spv"));

        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[]);

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });

        let workgroups: u32 = (x.len() as f32 / WORKGROUP_SIZE as f32).ceil() as u32;
        
        {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.dispatch(workgroups, 1, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }

        let command_buffer = encoder.finish();

        let start = Instant::now();

        queue.submit(Some(command_buffer));

        let buffer_slice = storage_buffer_outputs.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            println!("GPU: {} micros",start.elapsed().as_micros());

            let gpu_sum = f32::from_ne_bytes(data[0..4].try_into().unwrap());
            
            drop(data);
            storage_buffer_outputs.unmap();

            let start = Instant::now();

            let cpu_sum:f32 = x.iter().map(|x| x*x).sum::<f32>().sqrt();

            println!("CPU: {} micros",start.elapsed().as_micros());

            assert_eq!(gpu_sum,cpu_sum);
        }
    }
    // #[ignore] // This tests passes locally, yet for some dumb reason it fails on github actions, as such it is ignored to satisy ci
    #[actix_rt::test]
    async fn sasum_global() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (0..VECTOR_SIZE).map(|v| v as f32).collect();
        println!("x.len(): {}",x.len());

        let internal_size = (std::mem::size_of::<f32>() * x.len()/2) as wgpu::BufferAddress;
        println!("internal buffer size: {}",internal_size);

        let output_size = std::mem::size_of::<f32>() as wgpu::BufferAddress;
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_WRITE
        });
        let storage_buffer_internal = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Storage Buffer Internal"),
            size: internal_size,
            usage: wgpu::BufferUsage::STORAGE,
            mapped_at_creation: false,
        });
        let storage_buffer_outputs = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Storage Buffer Output"),
            size: output_size,
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_READ,
            mapped_at_creation: false,
        });

        let (bind_group,bind_group_layout) = get_compute_bind_group(&device,&[&storage_buffer_x,&storage_buffer_internal,&storage_buffer_outputs]);

        let shader = device.create_shader_module(wgpu::include_spirv!("../glsl/sasum_global.spv"));

        // Get compute pipeline with 1 u32/uint push constant
        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[std::mem::size_of::<u32>()]);

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });
        
        let workgroups: u32 = (x.len() as f32 / WORKGROUP_SIZE as f32).ceil() as u32;

        {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.set_push_constants(0u32, &[x.len() as u32]);
            cpass.dispatch(workgroups, 1, 1);
        }
        
        let command_buffer = encoder.finish();

        let start = Instant::now();

        queue.submit(Some(command_buffer));

        let buffer_slice = storage_buffer_outputs.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            println!("GPU: {} micros",start.elapsed().as_micros());

            let gpu_sum = f32::from_ne_bytes(data[0..4].try_into().unwrap());

            drop(data);
            storage_buffer_outputs.unmap();

            let start = Instant::now();
            let cpu_sum:f32 = x.into_iter().map(|x| x.abs()).sum();
            println!("CPU: {} micros",start.elapsed().as_micros());

            println!("gpu sum:\t{}",gpu_sum);
            println!("cpu sum:\t{}",cpu_sum);

            assert_eq!(gpu_sum,cpu_sum);
        }
    }
    #[ignore] // Fails due to lack of wgpu support for subgroups and/or floating point atomics
    #[actix_rt::test]
    async fn sasum_workgroup() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (0..VECTOR_SIZE).map(|_| 3f32).collect();
        println!("x.len(): {}",x.len());

        let output_size = std::mem::size_of::<f32>() as wgpu::BufferAddress;
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_WRITE
        });
        let storage_buffer_outputs = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Storage Buffer Output"),
            size: output_size,
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_READ,
            mapped_at_creation: false,
        });

        let (bind_group,bind_group_layout) = get_compute_bind_group(&device,&[&storage_buffer_x,&storage_buffer_outputs]);

        let shader = device.create_shader_module(wgpu::include_spirv!("../glsl/sasum_workgroup.spv"));

        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[]);

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });
        
        let workgroups: u32 = (x.len() as f32 / WORKGROUP_SIZE as f32).ceil() as u32;

        {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.dispatch(workgroups, 1, 1);
        }
        
        let command_buffer = encoder.finish();

        let start = Instant::now();

        queue.submit(Some(command_buffer));

        let buffer_slice = storage_buffer_outputs.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            println!("GPU: {} micros",start.elapsed().as_micros());

            let gpu_sum = f32::from_ne_bytes(data[0..4].try_into().unwrap());

            drop(data);
            storage_buffer_outputs.unmap();

            let start = Instant::now();
            let cpu_sum:f32 = x.iter().map(|x| x.abs()).sum();
            println!("CPU: {} micros",start.elapsed().as_micros());

            println!("gpu sum:\t{}",gpu_sum);
            println!("cpu sum:\t{}",cpu_sum);

            assert_eq!(gpu_sum,cpu_sum);
        }
    }
    #[ignore] // Fails due to lack of wgpu support for subgroups and/or floating point atomics
    #[actix_rt::test]
    async fn sasum_subgroup() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (0..VECTOR_SIZE).map(|_| 3f32).collect();
        println!("x.len(): {}",x.len());

        let output_size = std::mem::size_of::<f32>() as wgpu::BufferAddress;
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_WRITE
        });
        let storage_buffer_outputs = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Storage Buffer Output"),
            size: output_size,
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_READ,
            mapped_at_creation: false,
        });

        let (bind_group,bind_group_layout) = get_compute_bind_group(&device,&[&storage_buffer_x,&storage_buffer_outputs]);

        let shader = device.create_shader_module(wgpu::include_spirv!("../glsl/sasum_subgroup.spv"));

        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[]);

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });
        
        let workgroups: u32 = (x.len() as f32 / WORKGROUP_SIZE as f32).ceil() as u32;

        {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.dispatch(workgroups, 1, 1);
        }
        
        let command_buffer = encoder.finish();

        let start = Instant::now();

        queue.submit(Some(command_buffer));

        let buffer_slice = storage_buffer_outputs.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            println!("GPU: {} micros",start.elapsed().as_micros());

            let gpu_sum = f32::from_ne_bytes(data[0..4].try_into().unwrap());

            drop(data);
            storage_buffer_outputs.unmap();

            let start = Instant::now();
            let cpu_sum:f32 = x.iter().map(|x| x.abs()).sum();
            println!("CPU: {} micros",start.elapsed().as_micros());

            println!("gpu sum:\t{}",gpu_sum);
            println!("cpu sum:\t{}",cpu_sum);

            assert_eq!(gpu_sum,cpu_sum);
        }
    }
    #[ignore] // Fails due to lack of wgpu support for subgroups and/or floating point atomics
    #[actix_rt::test]
    async fn isamax() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (0..VECTOR_SIZE).map(|v| v as f32).collect();
        println!("x.len(): {}",x.len());

        let output_size = std::mem::size_of::<f32>() as wgpu::BufferAddress;
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_WRITE
        });
        let storage_buffer_outputs = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Storage Buffer Output"),
            size: output_size,
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_READ,
            mapped_at_creation: false,
        });

        let (bind_group,bind_group_layout) = get_compute_bind_group(&device,&[&storage_buffer_x,&storage_buffer_outputs]);

        let shader = device.create_shader_module(wgpu::include_spirv!("../glsl/sasum_subgroup.spv"));

        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[]);

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });
        
        let workgroups: u32 = (x.len() as f32 / WORKGROUP_SIZE as f32).ceil() as u32;

        {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.dispatch(workgroups, 1, 1);
        }
        
        let command_buffer = encoder.finish();

        let start = Instant::now();

        queue.submit(Some(command_buffer));

        let buffer_slice = storage_buffer_outputs.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            println!("GPU: {} micros",start.elapsed().as_micros());

            let gpu_max = f32::from_ne_bytes(data[0..4].try_into().unwrap());

            drop(data);
            storage_buffer_outputs.unmap();

            let start = Instant::now();
            let cpu_max: f32 = x.into_iter().max_by(|x,y| x.abs().partial_cmp(&y.abs()).unwrap()).unwrap();
            println!("CPU: {} micros",start.elapsed().as_micros());

            assert_eq!(gpu_max,cpu_max);
        }
    }

    #[ignore] // Fails due to lack of wgpu support for subgroups and/or floating point atomics
    #[actix_rt::test]
    async fn sgemv() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x: Vec<f32> = (0..MATRIX_SIZE).map(|_| 1f32).collect();
        println!("x.len(): {}",x.len());
        let y: Vec<f32> = (0..MATRIX_SIZE).map(|_| 1f32).collect();
        println!("y.len(): {}",y.len());
        #[allow(non_snake_case)]
        let A: Vec<f32> = (0..VECTOR_SIZE).map(|_| 1f32).collect();
        println!("A.len(): {}",A.len());

        let alpha:f32 = 1.;
        println!("alpha: {}",alpha);
        let beta:f32 = 1.;
        println!("beta: {}",beta);


        let value_staging_buffer_row_size:usize = MATRIX_SIZE * std::mem::size_of::<f32>();

        //println!("{} -> {}",value_staging_buffer_size,used_buffer_size);
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_WRITE
        });
        let storage_buffer_y = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer Y"),
            contents: bytemuck::cast_slice(&y),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_WRITE | wgpu::BufferUsage::MAP_READ
        });
        #[allow(non_snake_case)]
        let texture_A = device.create_texture(&wgpu::TextureDescriptor {
            label: Some("Texture A"),
            size: wgpu::Extent3d { width: MATRIX_SIZE as u32, height: MATRIX_SIZE as u32, depth:1 },
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: wgpu::TextureFormat::R32Float,
            usage: wgpu::TextureUsage::STORAGE | wgpu::TextureUsage::COPY_SRC | wgpu::TextureUsage::COPY_DST,
        });

        // Intialises A
        queue.write_texture(
            wgpu::TextureCopyViewBase {
                texture: &texture_A,
                mip_level: 0,
                origin: wgpu::Origin3d { x:0,y:0,z:0 }
            },
            bytemuck::cast_slice(&A),
            wgpu::TextureDataLayout { 
                offset: 0,
                bytes_per_row: value_staging_buffer_row_size as u32,
                rows_per_image: 0
            },
            wgpu::Extent3d { width: MATRIX_SIZE as u32, height: MATRIX_SIZE as u32, depth:1 }
        );

        let views:Vec<wgpu::TextureView> = vec![
            texture_A.create_view(&wgpu::TextureViewDescriptor::default())
        ];

        let bindgroup_layout_entries = vec![
            wgpu::BindGroupLayoutEntry {
                binding: 0,
                visibility: wgpu::ShaderStage::COMPUTE,
                ty: wgpu::BindingType::StorageBuffer {
                    dynamic: false,
                    readonly: false,
                    min_binding_size: None,
                },
                count: None,
            },
            wgpu::BindGroupLayoutEntry {
                binding: 1,
                visibility: wgpu::ShaderStage::COMPUTE,
                ty: wgpu::BindingType::StorageBuffer {
                    dynamic: false,
                    readonly: false,
                    min_binding_size: None,
                },
                count: None,
            },
            wgpu::BindGroupLayoutEntry {
                binding: 2,
                visibility: wgpu::ShaderStage::COMPUTE,
                ty: wgpu::BindingType::StorageTexture {
                    dimension: wgpu::TextureViewDimension::D2,
                    format: wgpu::TextureFormat::R32Float,
                    readonly: false,
                },
                count: None,
            }
        ];
        let bindgroup_entries = vec![
            wgpu::BindGroupEntry {
                binding: 0,
                resource: wgpu::BindingResource::Buffer(storage_buffer_x.slice(..)),
            },
            wgpu::BindGroupEntry {
                binding: 1,
                resource: wgpu::BindingResource::Buffer(storage_buffer_y.slice(..)),
            },
            wgpu::BindGroupEntry {
                binding: 2,
                resource: wgpu::BindingResource::TextureView(&views[0])
            }
        ];

        let bind_group_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
            label: None,
            entries: &bindgroup_layout_entries,
        });
        let bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: None,
            layout: &bind_group_layout,
            entries: &bindgroup_entries,
        });

        let shader = device.create_shader_module(wgpu::include_spirv!("../glsl/sgemv.spv"));

        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[]);

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });

        let x_workgroups: u32 = (x.len() as f32 / WORKGROUP_SIZE as f32).ceil() as u32;
        let y_workgroups: u32 = x.len() as u32;

        {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.dispatch(x_workgroups, y_workgroups, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }

        let start = Instant::now();

        queue.submit(Some(encoder.finish()));

        let buffer_slice = storage_buffer_y.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        //block_on(); // Blocks thread until buffer_future can be read
        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            println!("GPU: {} micros",start.elapsed().as_micros());

            let gpu_vec:Vec<f32> = data.chunks_exact(4).map(|b| f32::from_ne_bytes(b.try_into().unwrap())).collect();

            drop(data);
            storage_buffer_y.unmap();

            let start = Instant::now();
            let mut cpu_vec:Vec<f32> = vec!(0.;y.len());
            for y_indx in 0..y.len() {
                cpu_vec[y_indx] = x.iter().enumerate().map(|(indx,x_val)| {
                    *x_val * A[indx + y_indx * x.len()] * alpha
                }).sum();
                cpu_vec[y_indx] += beta * y[y_indx];
            }
            println!("CPU: {} micros",start.elapsed().as_micros());

            println!("gpu_vec.len(): {}",gpu_vec.len());
            println!("cpu_vec.len(): {}",cpu_vec.len());

            for (actual,expected) in gpu_vec.iter().zip(cpu_vec.iter()) {
                assert_eq!(actual,expected);
            }
        }
    }
   
    // Cannot be used with textures
    fn get_compute_bind_group(device: &wgpu::Device, buffers:&[&wgpu::Buffer]) -> (wgpu::BindGroup,wgpu::BindGroupLayout) {
        
        let descs:Vec<wgpu::BindGroupLayoutEntry> = buffers.iter().enumerate().map(|(indx,_)| get_buffer_BindGroupLayoutEntry(indx)).collect();


        let bind_group_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
            label: None,
            entries: &descs,
        });

        let descs:Vec<wgpu::BindGroupEntry> = buffers.iter().enumerate().map(|(indx,buffer)| get_buffer_BindGroupEntry(indx,buffer)).collect();
    
        // Instantiates the bind group, once again specifying the binding of buffers.
        let bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: None,
            layout: &bind_group_layout,
            entries: &descs,
        });
    
        return (bind_group,bind_group_layout);
        #[allow(non_snake_case)]
        fn get_buffer_BindGroupLayoutEntry(binding:usize) -> wgpu::BindGroupLayoutEntry {
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
        #[allow(non_snake_case)]
        fn get_buffer_BindGroupEntry(binding:usize,buffer:&wgpu::Buffer) -> wgpu::BindGroupEntry {
            wgpu::BindGroupEntry {
                binding: binding as u32,
                resource: wgpu::BindingResource::Buffer(buffer.slice(..)),
            }
        }
    }

    async fn get_compute_device_queue() -> (wgpu::Device,wgpu::Queue) {
        let instance = wgpu::Instance::new(wgpu::BackendBit::PRIMARY);
        let adapter = instance
            .request_adapter(&wgpu::RequestAdapterOptions {
                power_preference: wgpu::PowerPreference::HighPerformance,
                compatible_surface: None,
            })
            .await
            .unwrap();
    
        let mut limits = wgpu::Limits::default();
        limits.max_push_constant_size = 128u32;
    
        let (device, queue) = adapter
        .request_device(
            &wgpu::DeviceDescriptor {
                features: wgpu::Features::PUSH_CONSTANTS | wgpu::Features::MAPPABLE_PRIMARY_BUFFERS,
                limits: limits,
                shader_validation: true,
            },
            None,
        )
        .await
        .unwrap();
    
        return (device,queue);
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
    // For getting image row sizes
    // Rounds up `n` to the neatest multiple of `multiple`
    #[allow(unused)]
    fn round_up(n:usize,multiple:usize) -> usize {
        ((n+multiple-1) / multiple) * multiple
    }
}
