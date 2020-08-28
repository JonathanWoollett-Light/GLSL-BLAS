#[cfg(test)]
mod tests {
    use futures::executor::block_on;
    use wgpu::util::DeviceExt;
    use std::convert::TryInto;
    use itertools::izip;
    use std::time::Instant;

    const MATRIX_SIZE:usize = 1024;
    const VECTOR_SIZE:usize = MATRIX_SIZE * MATRIX_SIZE; 

    #[test]
    fn basic() {
        assert_eq!(4,2+2);
    }
    
    #[actix_rt::test]
    async fn sscal() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (0..VECTOR_SIZE).map(|v| v as f32).collect();

        println!("{}",x.len());

        let a:f32 = 2.;
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_READ | wgpu::BufferUsage::MAP_WRITE
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
            cpass.dispatch(x.len() as u32  / 1024, 1, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }

        let command_buffer = encoder.finish();

        let start = Instant::now();

        queue.submit(Some(command_buffer));

        let buffer_slice = storage_buffer_x.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);

        //let start = Instant::now();

        device.poll(wgpu::Maintain::Wait);

        //block_on(); // Blocks thread until buffer_future can be read
        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            println!("GPU: {} micros",start.elapsed().as_micros());

            let result:Vec<f32> = data.chunks_exact(4).map(|b| f32::from_ne_bytes(b.try_into().unwrap())).collect();

            drop(data);
            storage_buffer_x.unmap();

            let start = Instant::now();

            let cpu_vec:Vec<f32> = x.iter().map(|v| v*a).collect();
            println!("CPU: {} micros",start.elapsed().as_micros());

            println!("{} | {}",cpu_vec.len(),result.len()); // TODO Why is `result.len() == cpu_vec.len() + 1`, is it including the push constant?
            for (result,expected) in result.iter().zip(cpu_vec.iter()) {
                assert_eq!(result,expected);
            }
            //assert_eq!(result,cpu_vec);
        }
    }
    #[actix_rt::test]
    async fn saxpy() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (0..VECTOR_SIZE).map(|v| v as f32).collect();
        let y:Vec<f32> = (0..VECTOR_SIZE).map(|v| v as f32).collect();

        let a:f32 = 2.;

        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_WRITE // I beleive map write prevents creation of temporary staging buffer during initialization
        });
        let storage_buffer_y = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer Y"),
            contents: bytemuck::cast_slice(&y),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_READ | wgpu::BufferUsage::MAP_WRITE
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
            cpass.dispatch(x.len() as u32 / 1024, 1, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }

        let command_buffer = encoder.finish();

        let start =  Instant::now();

        queue.submit(Some(command_buffer));

        let buffer_slice = storage_buffer_y.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        //block_on(); // Blocks thread until buffer_future can be read
        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            println!("GPU: {} micros",start.elapsed().as_micros());

            let result:Vec<f32> = data.chunks_exact(4).map(|b| f32::from_ne_bytes(b.try_into().unwrap())).collect();

            drop(data);
            storage_buffer_y.unmap();

            let start = Instant::now();

            let cpu_vec:Vec<f32> = x.iter().zip(y.iter()).map(|(x,y)| y+x*a).collect();
            println!("CPU: {} micros",start.elapsed().as_micros());

            println!("{} | {}",cpu_vec.len(),result.len()); // TODO Why is `result.len() == cpu_vec.len() + 1`, is it including the push constant?
            for (result,expected) in result.iter().zip(cpu_vec.iter()) {
                assert_eq!(result,expected);
            }
            //assert_eq!(result,cpu_vec);
        }
    }
    #[actix_rt::test]
    async fn sdot() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (0..372).map(|v| v as f32).collect();
        let y:Vec<f32> = (0..372).map(|v| v as f32).collect();

        let slice_size = x.len() * std::mem::size_of::<f32>();
        let size = slice_size as wgpu::BufferAddress;
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x), // Casts [u32] to [u8]
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_WRITE
        });
        let storage_buffer_y = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer Y"),
            contents: bytemuck::cast_slice(&y), // Casts [u32] to [u8]
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_WRITE
        });
        let storage_buffer_outputs = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Storage Buffer Outputs"),
            size,
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_READ,
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
            cpass.dispatch((x.len() as f32 / 1024f32).ceil() as u32, 1, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }

        let command_buffer = encoder.finish();

        let start = Instant::now();

        queue.submit(Some(command_buffer));

        let buffer_slice = storage_buffer_outputs.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        //block_on(); // Blocks thread until buffer_future can be read
        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            println!("GPU: {} micros",start.elapsed().as_micros());
            
            let values = data.len() / 4;
            let number_of_partial_sums = (values as f32 / 1024f32).ceil() as usize; // Each workgroup produces 1 partial sum, each workgroup has up to 1024 values
            println!("{}->{} ({})",data.len(),values, number_of_partial_sums);

            let result:Vec<f32> = data[0..4*number_of_partial_sums].chunks_exact(4).map(|b| f32::from_ne_bytes(b.try_into().unwrap())).collect();
            println!("number of partial sums: {}",result.len());
            println!("1st partial sum: {}",result[0]);
            let final_sum:f32 = result.iter().sum();

            drop(data);
            storage_buffer_outputs.unmap();

            // println!("{:.?}",result);
            // println!("{:.?}",final_sum);

            let start = Instant::now();

            let cpu_vec:Vec<f32> = x.iter().zip(y.iter()).map(|(x,y)| y*x).collect();
            println!("number of vals: {}",cpu_vec.len());
            let cpu_vec:Vec<f32> = cpu_vec.chunks(1024).map(|chunk| chunk.iter().sum()).collect();
            println!("number of partial sums: {}",cpu_vec.len());
            println!("1st partial sum: {}",cpu_vec[0]);


            let cpu_vec:f32 = x.iter().zip(y.iter()).map(|(x,y)| x*y).sum();
            println!("CPU: {} micros",start.elapsed().as_micros());

            println!("{},{}",final_sum,cpu_vec);
            assert_eq!(final_sum,cpu_vec);
        }
    }
    #[actix_rt::test]
    async fn snrm2() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (0..VECTOR_SIZE).map(|v| v as f32).collect();

        let slice_size = x.len() * std::mem::size_of::<f32>();
        let size = slice_size as wgpu::BufferAddress;
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_WRITE
        });
        let storage_buffer_outputs = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Storage Buffer Outputs"),
            size,
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_READ,
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
            cpass.dispatch(x.len() as u32 / 1024, 1, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }

        let command_buffer = encoder.finish();

        let start = Instant::now();

        queue.submit(Some(command_buffer));

        let buffer_slice = storage_buffer_outputs.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        //block_on(); // Blocks thread until buffer_future can be read
        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            println!("GPU: {} micros",start.elapsed().as_micros());
            
            let number_of_vals = data.len()/1024+4;
            //println!("{}->{} ({})",data.len(),number_of_vals,number_of_vals/4);

            let result:Vec<f32> = data[0..number_of_vals].chunks_exact(4).map(|b| f32::from_ne_bytes(b.try_into().unwrap())).collect();
            let final_sum:f32 = result.iter().sum::<f32>().sqrt();

            drop(data);
            storage_buffer_outputs.unmap();

            // println!("{:.?}",result);
            // println!("{:.?}",final_sum);

            let start = Instant::now();

            let cpu_vec:f32 = x.iter().map(|x| x*x).sum::<f32>().sqrt();
            println!("CPU: {} micros",start.elapsed().as_micros());

            assert_eq!(final_sum,cpu_vec);
        }
    }
    #[actix_rt::test]
    async fn sasum() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (0-(VECTOR_SIZE/2)..VECTOR_SIZE-(VECTOR_SIZE/2)).map(|v| v as f32).collect();

        let slice_size = x.len() * std::mem::size_of::<f32>();
        let size = slice_size as wgpu::BufferAddress;
    
        let storage_buffer_x = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Storage Buffer X"),
            contents: bytemuck::cast_slice(&x),
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_WRITE
        });
        let storage_buffer_outputs = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Storage Buffer Outputs"),
            size,
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_READ,
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
            cpass.dispatch(x.len() as u32 / 1024, 1, 1);
        }

        let command_buffer = encoder.finish();

        let start = Instant::now();

        queue.submit(Some(command_buffer));

        let buffer_slice = storage_buffer_outputs.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        //block_on(); // Blocks thread until buffer_future can be read
        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            println!("GPU: {} micros",start.elapsed().as_micros());
            
            let number_of_vals = data.len()/1024+4;
            //println!("{}->{} ({})",data.len(),number_of_vals,number_of_vals/4);

            let result:Vec<f32> = data[0..number_of_vals].chunks_exact(4).map(|b| f32::from_ne_bytes(b.try_into().unwrap())).collect();
            let final_sum:f32 = result.iter().sum();

            drop(data);
            storage_buffer_outputs.unmap();

            // println!("{:.?}",result);
            // println!("{:.?}",final_sum);

            let start = Instant::now();

            let cpu_vec:f32 = x.iter().map(|x| x.abs()).sum::<f32>();
            println!("CPU: {} micros",start.elapsed().as_micros());

            assert_eq!(final_sum,cpu_vec);
        }
    }

    #[actix_rt::test]
    async fn sgemv() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (0..MATRIX_SIZE).map(|v| v as f32).collect();
        let y:Vec<f32> = (0..MATRIX_SIZE).map(|v| v as f32).collect();
        let A:Vec<f32> = (0..VECTOR_SIZE).map(|v| v as f32).collect();

        let alpha:f32 = 0.5;
        let beta:f32 = 0.1;

        let value_staging_buffer_size = A.len() * std::mem::size_of::<f32>();

        let value_staging_buffer_row_size:usize = MATRIX_SIZE * std::mem::size_of::<f32>();
        let used_buffer_row_size:usize = ((value_staging_buffer_row_size as f32 / 256.).ceil() * 256.) as usize;
        let used_buffer_size:usize = MATRIX_SIZE * used_buffer_row_size;
        let buffer_address_size = used_buffer_size as wgpu::BufferAddress;

        //println!("{} -> {}",value_staging_buffer_size,used_buffer_size);
    
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
        let texture_internal = device.create_texture(&wgpu::TextureDescriptor {
            label: Some("Texture Internal Holder"),
            size: wgpu::Extent3d { width: MATRIX_SIZE as u32, height: MATRIX_SIZE as u32, depth:1 },
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: wgpu::TextureFormat::R32Float,
            usage: wgpu::TextureUsage::STORAGE,
        });

        let texture_A = device.create_texture(&wgpu::TextureDescriptor {
            label: Some("Texture A"),
            size: wgpu::Extent3d { width: MATRIX_SIZE as u32, height: MATRIX_SIZE as u32, depth:1 },
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: wgpu::TextureFormat::R32Float,
            usage: wgpu::TextureUsage::STORAGE | wgpu::TextureUsage::COPY_SRC, // TODO Is `COPY_SRC` neccessary here?
        });

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

        let buffer_outputs = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Storage Buffer Outputs"),
            size: (MATRIX_SIZE * std::mem::size_of::<f32>()) as wgpu::BufferAddress,
            usage: wgpu::BufferUsage::STORAGE | wgpu::BufferUsage::MAP_READ,
            mapped_at_creation: false,
        });

        let views:Vec<wgpu::TextureView> = vec![
            texture_A.create_view(&wgpu::TextureViewDescriptor::default()),
            texture_internal.create_view(&wgpu::TextureViewDescriptor::default())
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
            },
            wgpu::BindGroupLayoutEntry {
                binding: 3,
                visibility: wgpu::ShaderStage::COMPUTE,
                ty: wgpu::BindingType::StorageBuffer {
                    dynamic: false,
                    readonly: false,
                    min_binding_size: None,
                },
                count: None,
            },
            wgpu::BindGroupLayoutEntry {
                binding: 4,
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
            },
            wgpu::BindGroupEntry {
                binding: 3,
                resource: wgpu::BindingResource::Buffer(buffer_outputs.slice(..)),
            },
            wgpu::BindGroupEntry {
                binding: 4,
                resource: wgpu::BindingResource::TextureView(&views[1])
            },
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

        let shader = device.create_shader_module(wgpu::include_spirv!("../spir-v/sgemv.spv"));

        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[2 * std::mem::size_of::<f32>()]);

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });

        unsafe {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.set_push_constants(0,std::mem::transmute(&[alpha,beta][..]));
            cpass.dispatch(MATRIX_SIZE as u32 / 32, MATRIX_SIZE as u32 / 32, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }

        let start = Instant::now();

        queue.submit(Some(encoder.finish()));

        let buffer_slice = buffer_outputs.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);
        device.poll(wgpu::Maintain::Wait);

        //block_on(); // Blocks thread until buffer_future can be read
        if let Ok(()) = buffer_future.await {
            let data = buffer_slice.get_mapped_range();
            println!("GPU: {} ms",start.elapsed().as_millis());

            // let rows_bytes:Vec<Vec<u8>> = data.chunks_exact(used_buffer_row_size as usize).map(|r| r[0..40].to_vec()).collect();
            // println!("[{},{}]",rows_bytes.len(),rows_bytes[0].len());
            // let rows_vals:Vec<Vec<f32>> = rows_bytes.iter().map(|r| r.chunks_exact(4).map(|v| f32::from_ne_bytes(v.try_into().unwrap())).collect()).collect();
            // println!("[{},{}]",rows_vals.len(),rows_vals[0].len());

            let full_result:Vec<f32> = data.chunks_exact(4).map(|b| f32::from_ne_bytes(b.try_into().unwrap())).collect();

            drop(data);
            buffer_outputs.unmap();

            //println!("full_result: {:.?}",full_result);
            //println!("length: {}",full_result.len());

            //println!("GPU: {} micros",start.elapsed().as_micros());

            let start = Instant::now();

            let mut cpu_vec:Vec<f32> = vec!(0.;MATRIX_SIZE);
            for y_indx in 0..MATRIX_SIZE {
                cpu_vec[y_indx] = x.iter().enumerate().map(|(indx,x)| x * (MATRIX_SIZE*y_indx + indx) as f32 * alpha).sum();
                cpu_vec[y_indx] += beta * y[y_indx];
            }
            println!("CPU: {} ms",start.elapsed().as_millis());

            assert_eq!(full_result,cpu_vec);
        }
    }
    #[actix_rt::test]
    async fn sgemv_short() {
        let (device,queue):(wgpu::Device,wgpu::Queue) = get_compute_device_queue().await;

        let x:Vec<f32> = (0..MATRIX_SIZE).map(|v| v as f32).collect();
        let y:Vec<f32> = (0..MATRIX_SIZE).map(|v| v as f32).collect();
        let A:Vec<f32> = (0..VECTOR_SIZE).map(|v| v as f32).collect();

        let alpha:f32 = 0.5;
        let beta:f32 = 0.1;

        let value_staging_buffer_size = A.len() * std::mem::size_of::<f32>();

        let value_staging_buffer_row_size:usize = MATRIX_SIZE * std::mem::size_of::<f32>();
        let used_buffer_row_size:usize = ((value_staging_buffer_row_size as f32 / 256.).ceil() * 256.) as usize;
        let used_buffer_size:usize = MATRIX_SIZE * used_buffer_row_size;
        let buffer_address_size = used_buffer_size as wgpu::BufferAddress;

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

        let texture_A = device.create_texture(&wgpu::TextureDescriptor {
            label: Some("Texture A"),
            size: wgpu::Extent3d { width: MATRIX_SIZE as u32, height: MATRIX_SIZE as u32, depth:1 },
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: wgpu::TextureFormat::R32Float,
            usage: wgpu::TextureUsage::STORAGE | wgpu::TextureUsage::COPY_SRC | wgpu::TextureUsage::COPY_DST, // TODO Does `COPY_DST` ned to be here?
        });
        let staging_texture_buffer = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Staging Texture Buffer"),
            size: buffer_address_size,
            usage: wgpu::BufferUsage::MAP_READ | wgpu::BufferUsage::COPY_DST,
            mapped_at_creation: false,
        });

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
            texture_A.create_view(&wgpu::TextureViewDescriptor::default()),
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

        let shader = device.create_shader_module(wgpu::include_spirv!("../spir-v/sgemv_short.spv"));

        let compute_pipeline = get_compute_pipeline(&device,bind_group_layout,shader,&[2 * std::mem::size_of::<f32>()]);

        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });

        //panic!("here?");

        unsafe {
            let mut cpass = encoder.begin_compute_pass();
            cpass.set_pipeline(&compute_pipeline);
            cpass.set_bind_group(0, &bind_group, &[]);
            cpass.set_push_constants(0,std::mem::transmute(&[alpha,beta][..]));
            cpass.dispatch(MATRIX_SIZE as u32 / 32, MATRIX_SIZE as u32 / 32, 1); // Number of cells to run, the (x,y,z) size of item being processed
        }
        encoder.copy_texture_to_buffer(
            wgpu::TextureCopyViewBase { texture: &texture_A, mip_level:0,origin: wgpu::Origin3d{x:0,y:0,z:0} },
            wgpu::BufferCopyViewBase { 
                buffer: &staging_texture_buffer,
                layout: wgpu::TextureDataLayout { offset:0, bytes_per_row: used_buffer_row_size as u32, rows_per_image: MATRIX_SIZE as u32}
            },
            wgpu::Extent3d { width: MATRIX_SIZE as u32, height: MATRIX_SIZE as u32, depth: 1}
        );

        let command_buffer = encoder.finish();

        let start = Instant::now();

        //panic!("here?");

        queue.submit(Some(command_buffer));

        let buffer_slice = storage_buffer_y.slice(..);
        let buffer_future = buffer_slice.map_async(wgpu::MapMode::Read);

        let texture_slice = staging_texture_buffer.slice(..);
        let texture_future = texture_slice.map_async(wgpu::MapMode::Read);

        device.poll(wgpu::Maintain::Wait);

        //block_on(); // Blocks thread until buffer_future can be read
        if let Ok(()) = buffer_future.await {
            if let Ok(()) = texture_future.await {
                // Running GPU
                let A = texture_slice.get_mapped_range();
                let rows_bytes:Vec<Vec<u8>> = A.chunks_exact(used_buffer_row_size as usize).map(|r| r[0..(MATRIX_SIZE * std::mem::size_of::<f32>())].to_vec()).collect();
                //println!("[{},{}]",rows_bytes.len(),rows_bytes[0].len());
                let rows_vals:Vec<Vec<f32>> = rows_bytes.iter().map(|r| r.chunks_exact(4).map(|v| f32::from_ne_bytes(v.try_into().unwrap())).collect()).collect();
                //println!("[{},{}]",rows_vals.len(),rows_vals[0].len());

                // println!("rows_vals:");
                // for line in rows_vals.iter() {
                //     println!("{:.?}",line);
                // }
                
                let y_data = buffer_slice.get_mapped_range();
                let new_y:Vec<f32> = y_data.chunks_exact(4).map(|b| f32::from_ne_bytes(b.try_into().unwrap())).collect();

                //println!("y: {:.?}",new_y);

                let mut new_A:Vec<f32> = vec!(0f32;MATRIX_SIZE);
                for y_indx in 0..MATRIX_SIZE {
                    new_A[y_indx] = rows_vals[y_indx].iter().sum::<f32>() + new_y[y_indx];
                }

                println!("GPU: {} ms",start.elapsed().as_millis());

                drop(y_data);
                storage_buffer_y.unmap();
                drop(A);
                staging_texture_buffer.unmap();

                //println!("full_result: {:.?}",full_result);
                //println!("length: {}",full_result.len());

                let start = Instant::now();

                // Running CPU
                //println!("cpu matrix:");
                let start = Instant::now();

                let mut cpu_vec:Vec<f32> = vec!(0.;MATRIX_SIZE);
                for y_indx in 0..MATRIX_SIZE {
                    cpu_vec[y_indx] = x.iter().enumerate().map(|(indx,x)| x * (MATRIX_SIZE*y_indx + indx) as f32 * alpha).sum();
                    cpu_vec[y_indx] += beta * y[y_indx];
                }
                println!("CPU: {} ms",start.elapsed().as_millis());

                assert_eq!(new_A,cpu_vec);
            }
        }
    }
   
    // Cannot be used with textures
    fn get_compute_bind_group(device: &wgpu::Device, buffers:&[&wgpu::Buffer]) -> (wgpu::BindGroup,wgpu::BindGroupLayout) {
        
        let mut descs:Vec<wgpu::BindGroupLayoutEntry> = buffers.iter().enumerate().map(|(indx,_)| get_buffer_BindGroupLayoutEntry(indx)).collect();


        let bind_group_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
            label: None,
            entries: &descs,
        });

        let mut descs:Vec<wgpu::BindGroupEntry> = buffers.iter().enumerate().map(|(indx,buffer)| get_buffer_BindGroupEntry(indx,buffer)).collect();
    
        // Instantiates the bind group, once again specifying the binding of buffers.
        let bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: None,
            layout: &bind_group_layout,
            entries: &descs,
        });
    
        return (bind_group,bind_group_layout);
    
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
        fn get_texture_BindGroupLayoutEntry(binding:usize) -> wgpu::BindGroupLayoutEntry {
            wgpu::BindGroupLayoutEntry {
                binding: binding as u32,                             // The location
                visibility: wgpu::ShaderStage::COMPUTE, // Which shader type in the pipeline this buffer is available to.
                ty: wgpu::BindingType::StorageTexture {
                    dimension: wgpu::TextureViewDimension::D2,
                    format: wgpu::TextureFormat::R32Float, // Specifies if the buffer can only be read within the shader
                    readonly: false,
                },
                count: None,
            }
        }
        fn get_buffer_BindGroupEntry(binding:usize,buffer:&wgpu::Buffer) -> wgpu::BindGroupEntry {
            wgpu::BindGroupEntry {
                binding: binding as u32,
                resource: wgpu::BindingResource::Buffer(buffer.slice(..)),
            }
        }
        // fn get_texture_BindGroupEntry(binding:usize,texture:&wgpu::Texture,view:TextureView) -> wgpu::BindGroupEntry {
        //     wgpu::BindGroupEntry {
        //         binding: binding as u32,
        //         resource: wgpu::BindingResource::TextureView(view)
        //     }
        // }
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
}