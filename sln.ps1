$has_include = (Join-Path (Get-Item .).Parent.FullName has_include)
$preprocessor = (Join-Path (Get-Item .).Parent.FullName preprocessor)
$apply = (Join-Path (Get-Item .).Parent.FullName apply)
$asio = (Join-Path (Get-Item .).Parent.FullName asio-1.18.0)
$dbg_macro = (Join-Path (Get-Item .).Parent.FullName dbg-macro)
cmake -B "build" -G "Visual Studio 16 2019" -A x64 -T v142 -Dhas_include_ROOT="$has_include" -Dpreprocessor_ROOT="$preprocessor" -Dapply_ROOT="$apply" -Dasio_ROOT="$asio" -Ddbg_macro_ROOT="$dbg_macro"