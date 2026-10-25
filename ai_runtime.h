#pragma once

#include <string>
#include <vector>

// Runtime entry points used by generated StoryLang assembly.
// Arguments are passed as: count, prompt, option1, option2, ...
extern "C" int story_ai_choose(int argumentCount, ...);
extern "C" void story_ai_generate(int argumentCount, ...);

int story_ai_choose_values(const std::vector<std::string> &values);
void story_ai_generate_values(const std::vector<std::string> &values);
