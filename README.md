# 🚧 EXTRME WIP 🚧

Come back in a couple months.

Everything but the GLSL files is a mess (and even the GLSL files are still rough).

## Report

<table>
<tr><th>Operation support</th><th>Key</th></tr>
<tr><td>

Level 1 |  | Level 2 |  | Level 3 |  |
--- | --- | --- | --- | --- | --- |
sscal | ✅ | sgemv | ✔️ | sgemm | 📅 |
saxpy | ✅ |  |   |   |   | 
sdot | ✔️ |  |   |   |   | 
snrm2 | ✔️ |  |   |   |   | 
sasum | ✔️ |  |   |   |   |
isamax | ✔️ |  |   |   |   | 

</td><td>

Sy | Desc
--- | ---
☑️ | Optimized
✅ | Tested
✔️ | Designed
📅 | Planned

</td></tr> </table>

## Support
Library |  GL_EXT_shader_atomic_float | GL_KHR_shader_subgroup_arithmetic
--- | --- | ---
[WebGPU](https://github.com/gfx-rs/wgpu) | ❌ | ✔️
[Vulkano](https://github.com/vulkano-rs/vulkano) | ❌ | ❌
