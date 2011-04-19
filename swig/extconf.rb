require 'mkmf'
`swig -w801 -c++ -ruby jackcpp.i`
$libs = append_library($libs, "stdc++")
$libs = append_library($libs, "jackcpp")
$libs = append_library($libs, "jack")
dir_config(".", ["../include/"], "..")
create_makefile('jackaudio')
