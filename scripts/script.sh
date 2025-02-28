#!/bin/bash
#
# Build and run doowm in an Xephyr instance.

set -e
cd "$(dirname "$0")/.."
PROJECT_ROOT=$(pwd)
echo "Project root: $PROJECT_ROOT"

# デバッグモードのフラグ（デフォルトはオフ）
DEBUG_MODE=0

# コマンドライン引数の処理
while [[ $# -gt 0 ]]; do
  case $1 in
    --debug)
      DEBUG_MODE=1
      shift
      ;;
    *)
      shift
      ;;
  esac
done

# 1. Get current monitor resolution
if command -v xrandr >/dev/null 2>&1; then
    # Get primary monitor dimensions using xrandr
    MONITOR_INFO=$(xrandr --current | grep -w connected | grep primary || xrandr --current | grep -w connected | head -n 1)
    if [[ $MONITOR_INFO =~ ([0-9]+)x([0-9]+)\+[0-9]+\+[0-9]+ ]]; then
        MONITOR_WIDTH=${BASH_REMATCH[1]}
        MONITOR_HEIGHT=${BASH_REMATCH[2]}
    else
        # Try alternative pattern
        MONITOR_INFO=$(xrandr --current | grep -A1 -w connected | grep -w "[0-9]\+x[0-9]\+" | head -n 1)
        if [[ $MONITOR_INFO =~ ([0-9]+)x([0-9]+) ]]; then
            MONITOR_WIDTH=${BASH_REMATCH[1]}
            MONITOR_HEIGHT=${BASH_REMATCH[2]}
        else
            # Default fallback size
            MONITOR_WIDTH=1920
            MONITOR_HEIGHT=1080
        fi
    fi
    
    # Use full monitor size for Xephyr
    XEPHYR_WIDTH=$((MONITOR_WIDTH))
    XEPHYR_HEIGHT=$((MONITOR_HEIGHT))
else
    # Default fallback size
    XEPHYR_WIDTH=1280
    XEPHYR_HEIGHT=800
fi

echo "Using Xephyr resolution: ${XEPHYR_WIDTH}x${XEPHYR_HEIGHT}"

# 2. Build binary with debug symbols
BUILD_DIR="$PROJECT_ROOT/build"
mkdir -p "$BUILD_DIR"

# CMakeでビルドディレクトリを指定（デバッグモード）
if [ "$DEBUG_MODE" -eq 1 ]; then
    echo "Building in debug mode..."
    cmake -B "$BUILD_DIR" -S "$PROJECT_ROOT" -DCMAKE_BUILD_TYPE=Debug
else
    cmake -B "$BUILD_DIR" -S "$PROJECT_ROOT"
fi

# makeでビルドディレクトリを指定してビルド
make -C "$BUILD_DIR" -j$(nproc)

# 3. Check if binary exists
if [ ! -f "$BUILD_DIR/doowm" ]; then
    echo "Error: Build failed, binary not found"
    exit 1
fi

# 4. Check for Xephyr
XEPHYR=$(whereis -b Xephyr | cut -d' ' -f2)
if [ -z "$XEPHYR" ]; then
    echo "Xephyr not found. Please install xserver-xephyr"
    exit 1
fi

# 5. Kill any existing Xephyr processes
killall Xephyr 2>/dev/null || true

# 6. Create a unique display number
DISPLAY_NUM=42
while [ -e "/tmp/.X${DISPLAY_NUM}-lock" ]; do
    DISPLAY_NUM=$((DISPLAY_NUM + 1))
done

# 7. Start Xephyr with detected monitor size
$XEPHYR :${DISPLAY_NUM} \
    -br \
    -ac \
    -noreset \
    -screen ${XEPHYR_WIDTH}x${XEPHYR_HEIGHT} &
XEPHYR_PID=$!

# 8. Wait for Xephyr to start
sleep 1

# 9. Export the display for xinitrc
export DISPLAY=:${DISPLAY_NUM}

# デバッグモードの場合、環境変数を設定
if [ "$DEBUG_MODE" -eq 1 ]; then
    export USE_GDB=1
    
    # GDBコマンドファイルの作成
    GDB_CMD_FILE="$PROJECT_ROOT/gdb_commands.txt"
    cat > "$GDB_CMD_FILE" << EOF
set pagination off
set logging on gdb_output.log
set logging overwrite on
break main
break X::X::initialize
break X::EventHandler::processNextEvent
break X::Launcher::createWindow
break X::Launcher::show
break Logger::init
run
EOF
    
    # xinitrcを起動（バックグラウンドで）
    chmod +x "$PROJECT_ROOT/scripts/xinitrc"
    "$PROJECT_ROOT/scripts/xinitrc" &
    XINIT_PID=$!
    
    echo "Starting GDB..."
    cd "$PROJECT_ROOT"
    
    # 新しいターミナルでGDBを起動
    if command -v gnome-terminal >/dev/null 2>&1; then
        gnome-terminal -- gdb -x "$GDB_CMD_FILE" "$BUILD_DIR/doowm"
    elif command -v xterm >/dev/null 2>&1; then
        xterm -e "gdb -x \"$GDB_CMD_FILE\" \"$BUILD_DIR/doowm\""
    else
        echo "No terminal emulator found. Running GDB in current terminal."
        gdb -x "$GDB_CMD_FILE" "$BUILD_DIR/doowm"
    fi
else
    # 通常モードの場合、xinitrcを実行
    chmod +x "$PROJECT_ROOT/scripts/xinitrc"
    "$PROJECT_ROOT/scripts/xinitrc" &
    XINIT_PID=$!
fi

# 10. Wait for user interrupt
echo "Xephyr running on display :${DISPLAY_NUM}"
echo "Press Ctrl+C to exit"
trap 'kill $XEPHYR_PID $XINIT_PID 2>/dev/null; exit 0' INT TERM
wait $XINIT_PID

# 11. Cleanup
kill $XEPHYR_PID 2>/dev/null || true
