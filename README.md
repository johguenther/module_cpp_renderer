Run the GLUT viewer with...

```./ospGlutViewer [model file] -m cpp -c cpp_perspective --trianglemesh-type cpp_triangles -r cpp_ao_stream```

Where valid renderers are:

- cpp_raycast
- cpp_raycast_stream
- cpp_ao
- cpp_ao_stream
- cpp_scivis
- cpp_scivis_stream


SIMD rendering using boost.simd is being developed, where the following renderers are defined:

- cpp_raycast_simd
- cpp_ao_simd

NOTE: SIMD renderers need a SIMD camera, such as cpp_perspective_simd

Alternatively, you can run 'ospCppViewer' directly, which by default adds
everything you need by default (including module load). You can then override
any object type, such as the renderer, just as you would do so in the GLUT viewer.
