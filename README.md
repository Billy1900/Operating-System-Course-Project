# Operating-System-Course-Project
华中科技大学操作系统课设2020
## Report
- [Report](https://github.com/Billy1900/Operating-System-Course-Project/blob/master/lnq.pdf)


## Issues
- [GTK+3 tutorial](https://developer.gnome.org/gtk3/stable/gtk-getting-started.html)
- [Ubuntu下如何安装GTK+3](https://www.jianshu.com/p/64ca37b99244)
- [GDK3 tutorial](https://developer.gnome.org/gdk3/stable/)
- [Migrating from GTK+ 2.x to GTK+ 3](https://developer.gnome.org/gtk3/stable/gtk-migrating-2-to-3.html)
- How to link gtk 3 in clion?

  write code in cmake as following:
<pre>
cmake_minimum_required(VERSION 3.2)
project(SimpleCalculatorPro)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

add_definitions(${GTK3_CFLAGS_OTHER})

add_executable(SimpleCalculatorPro SCP.cpp Calculator.cpp Calculator.h Arithmetics.cpp Arithmetics.h History.cpp History.h Interface.cpp Interface.h)
target_link_libraries(SimpleCalculatorPro ${GTK3_LIBRARIES})
</pre>


## Supplementary
- [《一个64位操作系统的设计与实现》学习笔记](https://github.com/yifengyou/The-design-and-implementation-of-a-64-bit-os)
