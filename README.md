# C++ Weather Studio

C++ Weather Studio is a desktop weather application built with **C++**, **Dear ImGui**, **GLFW**, and **OpenGL**. It fetches weather data from **Open-Meteo**, supports both **city/state** and **latitude/longitude** input, lets you request a **date range**, keeps a **local query history**, and renders a simple animated weather panel inside the UI.

This is a real desktop GUI app rather than a console demo. It is designed to be readable, interactive, and practical for learning how networking, JSON parsing, GUI rendering, and app-state management fit together in C++.

## Features

- Desktop GUI built with Dear ImGui + GLFW + OpenGL
- Search by:
  - **City / State**
  - **Latitude / Longitude**
- Built-in quick-select buttons for:
  - Phoenix, AZ
  - New York, NY
  - Los Angeles, CA
  - Chicago, IL
  - Dallas, TX
  - Seattle, WA
- Start and end date inputs using `YYYY-MM-DD`
- **Today** and **Use Today for Both** shortcuts
- Automatic city/state geocoding through Open-Meteo
- Forecast + archive date-range support
- Current temperature and “feels like” display when available
- Daily results table with:
  - high / low temperatures
  - precipitation
  - wind speed
  - weather code description
- Animated weather visualization with:
  - Auto
  - Sunny
  - Rain
  - Snow
  - Cloud
- Local query history saved to JSON
- CSV export for saved history
- Adjustable text size / text scale in the UI
- Async background fetch with loading spinner

## How it Works

The application runs a GLFW window, initializes Dear ImGui, and renders the app each frame. When the user requests weather data, the app validates inputs, optionally geocodes the city/state into coordinates, then calls Open-Meteo for weather data. Results are displayed in the GUI and also appended to a local history log.

The weather service handles three date-range cases:

1. **Past only** → uses the Open-Meteo archive endpoint
2. **Today / future only** → uses the forecast endpoint
3. **Range crossing today** → combines archive data for the past slice with forecast data for the current/future slice

## Dependencies

You will need these libraries available on your system:

- **C++17** compiler or newer
- **GLFW**
- **OpenGL**
- **libcurl**
- **nlohmann/json**
- **Dear ImGui**
- Dear ImGui backends:
  - `imgui_impl_glfw`
  - `imgui_impl_opengl2`


## Build Notes

This project is not a single-file compile. In addition to your application `.cpp` files, you also need to compile the Dear ImGui source files and backend files.

Typical ImGui source files needed:

- `imgui.cpp`
- `imgui_draw.cpp`
- `imgui_tables.cpp`
- `imgui_widgets.cpp`
- `backends/imgui_impl_glfw.cpp`
- `backends/imgui_impl_opengl2.cpp`


## Example g++ Build Command

The exact include and library paths depend on where you installed GLFW, OpenGL, libcurl, nlohmann/json, and Dear ImGui.

A typical build command looks like this:

```bash
g++ -std=c++17 -O2 ^
  main.cpp App.cpp DateUtils.cpp GeocodingService.cpp HistoryService.cpp HttpClient.cpp WeatherCodeMap.cpp WeatherService.cpp AnimationRenderer.cpp ^
  imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp ^
  backends/imgui_impl_glfw.cpp backends/imgui_impl_opengl2.cpp ^
  -I. -Ipath/to/imgui -Ipath/to/imgui/backends ^
  -lglfw -lopengl32 -lcurl ^
  -o weather_studio.exe
```

## Example Build on MSYS2 (UCRT64)

If you are building on Windows with **MSYS2 UCRT64**, install the common dependencies first:

```bash
pacman -S --needed \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-glfw \
  mingw-w64-ucrt-x86_64-curl \
  mingw-w64-ucrt-x86_64-nlohmann-json
```

Then compile from the UCRT64 shell, adjusting the ImGui include paths to match your local layout:

```bash
g++ -std=c++17 -O2 \
  main.cpp App.cpp DateUtils.cpp GeocodingService.cpp HistoryService.cpp HttpClient.cpp WeatherCodeMap.cpp WeatherService.cpp AnimationRenderer.cpp \
  imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp \
  backends/imgui_impl_glfw.cpp backends/imgui_impl_opengl2.cpp \
  -I. -I./imgui -I./imgui/backends \
  -lglfw -lopengl32 -lcurl \
  -o weather_studio.exe
```

## Run

After a successful build:

```bash
./weather_studio.exe
```

Or on Linux-like shells:

```bash
./weather_studio
```

## Usage

1. Launch the app
2. Choose an input mode:
   - **City / State**
   - **Latitude / Longitude**
3. Enter a location or click one of the quick city buttons
4. Enter:
   - `Start Date (YYYY-MM-DD)`
   - `End Date (YYYY-MM-DD)`
5. Click **Get Weather**
6. Review:
   - current conditions
   - daily forecast/history rows
   - weather animation
   - saved history entries

## Files Created at Runtime

The code creates and uses these local files:

- `startup.log`  
  Used for GLFW startup and error logging.

- `weather_history.json`  
  Stores saved query history between runs.

- `weather_history_export.csv`  
  Created when you click **Export CSV** in the History panel.

## Input Validation

The app validates:

- ISO date format: `YYYY-MM-DD`
- start date must not be after end date
- city/state cannot be empty in city/state mode
- latitude and longitude must be numeric in coordinate mode
- latitude must be between `-90` and `90`
- longitude must be between `-180` and `180`

## Weather Data Returned

The daily data model includes fields such as:

- date
- weather code
- description
- daily high temperature
- daily low temperature
- apparent high / low temperature
- precipitation total
- max wind speed
- source (`archive` or `forecast`)

When forecast data is available, the app also shows current:

- temperature
- apparent temperature

## Animation Modes

The animation panel supports manual override and automatic mode selection.

Manual modes:

- Auto
- Sunny
- Rain
- Snow
- Cloud

In **Auto**, the app maps the first returned weather code to a visual mode.


## APIs Used

- **Open-Meteo Geocoding API**
- **Open-Meteo Forecast API**
- **Open-Meteo Archive API**

## Troubleshooting

### Window fails to open
Check `startup.log` for GLFW-related startup errors.

### Network request fails
The HTTP client uses libcurl and includes a retry path that forces IPv4 if a connection fails on some Windows networks.

### No results returned
Verify:

- the city/state is valid
- the coordinates are valid
- the selected date range is available from the API
- your internet connection is working

### Build errors for ImGui backend headers
Make sure your include paths contain both:

- the Dear ImGui root directory
- the `backends` directory

## License
```text
MIT License
```
