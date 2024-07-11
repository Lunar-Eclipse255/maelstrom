# maelstrom

<img src="https://img.shields.io/badge/version-v0.1.0-blue?style=for-the-badge">

### Intro
maelstrom is a [PROS](https://pros.cs.purdue.edu/) library

## Installing maelstrom library
1. In the integrated PROS terminal, run the command `pros c add-depot maelstrom https://lunar-eclipse255.github.io/maelstrom/template.json`

2.  `cd` into your pros project

3.  Make sure you are in the kernwl version 4.1.0, your version can be found out with `pros c info-project` 

4. Apply the library to the project with `pros c apply maelstrom`

7. Put `#include "maelstrom/api.hpp"` in your main.h

## Alternative instructions for installing maelstrom library


1.  To get the library/template, head over to the releases page and download maelstrom@<version>.zip

2. Move the zip file to the pros project

3.  Make sure you are in the kernwl version 4.1.0, your version can be found out with `pros c info-project` 

4. Run `pros c fetch maelstrom@<version>.zip` in order to import the template

5. Apply the library to the project with `pros c apply maelstrom`

6. Put `#include "maelstrom/api.hpp"` in your main.h
