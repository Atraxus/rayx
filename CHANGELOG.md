# Changes

- rayx-core:
  - Added optical elements
    - Paraboloid 
  - Added light sources
    - Simple Undulator
    - Pixel Source
    - Dipole Source
  - Improved code consistency for optical elements and light sources
  - Improved test suite
  - Fixes for RZP tracing
- rayx:
  - Added more options to customize the export of rays and speed up the tracing
  - Added option to choose GPU
  - Improved runtime by optimizing data handling
- rayx-ui:
  - Added orthographic camera
  - Added footprints/heatmaps/histograms for simple optical elements
  - Changed default camera position and orientation
  - Jump to light source is possible now
  - Correctly render slits
  - Fixed positioning of light sources
  - Fixed triangulation bugs
  - Fixed general rendering bugs
- General bugfixes and documentation updates