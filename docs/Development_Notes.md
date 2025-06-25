# General
- Install btop to track cpu and gpu usage
- Try different compiler, and different options
- Use Release with CMake to benchmark
- Performances profiling to see which part of the code needs to be improved (Maqao, Valgrind) and Memory Profiling (MALT)

# Mathematics

- Try to add constexpr and noexcept in functions definition 
- Be careful about the purity of the functions (same inputs => same outputs)
- Avoid division : compute the inverse of the factor one time and then multiply by it
- Store result of compution (like compute inverse), instead of running that several times
 
- **Vector :**
  - Try to use 3 decimals instead of an array 
- **Matrix :**
  - Try to use 9 decimals instead of an array of 3 Vector3D
  - Verify normalize method : currently, it is using Gram-Schmidt orthonormalisation. Check if this method is well suited for my application
  - Add test for normalization
- **Quaternion :**
  - Try to use 4 decimals instead of one Vector3D and one decimal
