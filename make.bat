mkdir bin
cl /LD .\src\monitor.cpp user32.lib -DUNICODE /Fo:".\\bin\\" /Fe:".\\bin\\"
cl .\src\main.cpp user32.lib -DUNICODE /EHsc /Fo:".\\bin\\" /Fe:".\\bin\\"