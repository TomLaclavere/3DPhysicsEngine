# General
- Use a lot of assert to test at the beginning of methods (assert are disabled in Release compilation mode, they are only seen in Debug)
- Try different compiler, and different options
- Use Release with CMake to benchmark
- Performances profiling to see which part of the code needs to be improved (Maqao, Valgrind) and Memory Profiling (MALT)
- Be careful about all the struct, which can increase the compilation time (look how Chrono deal with that / use external solution for mathematical functions)
  - In particular, using struct for basic mathematical functions may be a wrong choice compare to using standard library or external optimized library
- Be careful about using WSL, which might kill the performances
- **Always keep the "naive" version !!! **
  - To be sure that the optimised version is consistent
  - To compare the performances

# Mathematics

- Try to add constexpr and noexcept in functions definition 
- Be careful about the purity of the functions (same inputs => same outputs)
- Avoid division : compute the inverse of the factor one time and then multiply by it
- Store result of computation (like compute inverse), instead of running that several times
 
- **Vector :**
  - Try to use 3 decimals instead of an array 
- **Matrix :**
  - Try to use 9 decimals instead of an array of 3 Vector3D
  - Add test for normalization        
  - Verify normalize method : currently, it is using Gram-Schmidt orthonormalisation. Check if this method is well suited for my application
- **Quaternion :**
  - Try to use 4 decimals instead of one Vector3D and one decimal
