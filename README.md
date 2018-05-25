# C Minifier

Usage: minify input.c [-h header.h] > output.c

Use tools like sed to remove unnecessary whitespace and comments.

To keep certain identifiers intact, you can list them line by line in a file
called .minify\_blacklist in the working directory. You can also use the -h
command line option to make the minifier parse and blacklist all the identifiers
stb\_c\_lexer can find in desired header files.

## Known issues:

- using a shorter id in the source than the length of current alias can result
  in name collisions in the output
