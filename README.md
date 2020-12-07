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
Library |  `GL_EXT_shader_atomic_float` | `GL_KHR_shader_subgroup_arithmetic`
--- | --- | ---
[WebGPU](https://github.com/gfx-rs/wgpu) | ❌ | ✔️
[Vulkano](https://github.com/vulkano-rs/vulkano) | ❌ | ❌

- `GL_EXT_shader_atomic_float`: Inter-workgroup reduction. To return 1 sum, max, etc. instead of 1 for each workgroup.
- `GL_KHR_shader_subgroup_arithmetic`: Fast intra-workgroup reduction. To get sum, max, etc. within a workgroup quickly.

Both can be worked around, but awkwardly.

Neither are new, code shouldn't need to be made worse due to the lack of support here.

Floating point atomic operations in particular are very fundemental compute operations, while lack of support for subgroup operations is dissapointing, lack of support for floating point atomics is egregious for any compute framework.
