# GLSL BLAS

GLSL implemented with Vulkan for BLAS operations.

## Report

<table>
<tr><th>Operation support</th><th>Key</th></tr>
<tr><td>

Level 1 |  | Level 2 |  | Level 3 |  |
--- | --- | --- | --- | --- | --- |
sscal & dscal | ✅ | sgemv & dgemv | ✅ | sgemm & dgemm | ✅ |
saxpy & daxpy | ✅ |  |   |   |   | 
sdot & ddot | ✅ |  |   |   |   | 
snrm2 & dnrm2 | ✅ |  |   |   |   | 
sasum & dasum | ✅ |  |   |   |   |
isamax & idamax | ✅ |  |   |   |   | 

</td><td>

Sy | Desc
--- | ---
✅ | Tested
✔️ | Designed
📅 | Planned

</td></tr> </table>

## Support

Your GPU likely supports subgroups operations, but likely does not support float atomics ([list of GPUs which support float atomics](https://vulkan.gpuinfo.org/listdevicescoverage.php?extension=VK_EXT_shader_atomic_float)), this is why I don't use them.

- `GL_EXT_shader_atomic_float`: Inter-workgroup reduction. To return 1 sum, max, etc. instead of 1 for each workgroup.
- `GL_KHR_shader_subgroup_arithmetic`: Fast intra-workgroup reduction. To get sum, max, etc. within a workgroup quickly.
