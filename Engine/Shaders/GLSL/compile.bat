if not exist ".\bin" mkdir .\bin

for %%f in (.\src\*) do (

  echo %%~xf
  
  if NOT "%%~xf" == ".glsl" (
    (( %VULKAN_SDK%/Bin/glslc.exe %%f -o .\bin\%%~nxf ) && ( echo Compile Successful )) || pause 
  )
)
