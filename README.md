## CMake

1. Change project name on line 26
2. Add libraries on line 34/35
3. Link libraries on line 78

When creating new files in VS, don't add to CMake when prompt pops up

To switch to building production build, change OFF to ON on line 8

To check for build mode in code, use e.g. PRODUCTION_BUILD == 0 in macros

To open resources, use RESOURCES_PATH macro where you want to open a resource from the resources folder e.g. open(RESOURCES_PATH "file.txt")

## Notes

https://thelinuxcode.com/socket-programming-in-c-building-networked-applications-from-the-ground-up/
https://sookocheff.com/post/networking/how-do-websockets-work/
