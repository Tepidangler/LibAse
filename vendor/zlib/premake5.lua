externalproject "zlibvc"
    location "./contrib/vstudio/vc17"
    uuid "8FD826F8-3739-44E6-8CC8-997122E53B8D"
    kind "SharedLib"
    language "C"

    defines
    {
        ZLIB_WINAPI
    }
    
    postbuildcommands
    {
        ("{COPY} C:/Windows/System32" )
    }