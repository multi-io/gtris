#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include "TetrisGameProcess.h"

bool GetOptions (unsigned* level,
                 std::string* hscfile,
                 CTetrisGameProcess::StoneColorRange* colorRange,
                 unsigned* brickSize);


#endif
