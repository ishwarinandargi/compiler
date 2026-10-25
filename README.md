# StoryLang

## Build

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build -j 2
ctest --test-dir build --output-on-failure
```

## AI calls

StoryLang supports these runtime calls:

```story
STATE choice: {attack | flee}
INIT choice := AI.choose("What should the hero do?", "attack", "flee")
ACTION AI.generate("Describe the next scene")
```

For offline tests:

```powershell
$env:STORYLANG_AI_MOCK = "1"
.\build\bin\storylang_cli.exe .\ai_demo.slt --run
```

`--run` executes the StoryLang AST and prints `AI.generate` output in the terminal. Mock mode prints the prompt without making a network request.

For real Gemini requests, add these variables to `.env` or set them in PowerShell:

```powershell
$env:GEMINI_API_KEY="your_gemini_api_key_here"
$env:GEMINI_MODEL="your_gemini_model_name"
```

The generated target code still calls `story_ai_choose` and `story_ai_generate` from `storylang_runtime`; only the provider used by that runtime has changed to Gemini.
