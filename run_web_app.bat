@echo off
title String Matching Algorithm Visualizer ^& Benchmark Suite
echo =====================================================================
echo    STARTING STRING MATCHING ALGORITHM VISUALIZER ^& BENCHMARK SUITE
echo    Algorithms: Naive ^| Rabin-Karp ^| Finite Automata ^| KMP
echo =====================================================================
echo.

cd /d "%~dp0backend"
echo [1/2] Launching Python Flask Backend Server on port 5000...
start "" http://localhost:5000
python app.py

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Python failed to start the server.
    echo Please make sure Python is installed and Flask is available.
    pause
)
