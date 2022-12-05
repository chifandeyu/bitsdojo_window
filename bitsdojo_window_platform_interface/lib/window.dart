import 'dart:ui';
import 'package:flutter/painting.dart';

enum WindowEventType {
  unknownType,
  // 进入全屏
  enterFullScreen,
  // 退出全屏
  leaveFullScreen,
  // 显示鼠标光标
  showCursor,
  // 隐藏鼠标光标
  hideCursor,
}

typedef WindowEventCallback = void Function(WindowEventType eventType);

abstract class DesktopWindow {
  DesktopWindow();
  int? get handle;
  double get scaleFactor;

  Rect get rect;
  set rect(Rect newRect);

  Offset get position;
  set position(Offset newPosition);

  Size get size;
  set size(Size newSize);

  set minSize(Size? newSize);
  set maxSize(Size? newSize);

  Alignment? get alignment;
  set alignment(Alignment? newAlignment);

  set title(String newTitle);

  @Deprecated("use isVisible instead")
  bool get visible;
  bool get isVisible;
  @Deprecated("use show()/hide() instead")
  set visible(bool isVisible);
  void show();
  void hide();
  void close();
  void minimize();
  void maximize();
  void maximizeOrRestore();
  void restore();
  bool isFullScreen();
  void showFullScreen(bool isFullScreen);
  void startDragging();
  // Add window event listener for window changed, e.g. enter full screeb, leave full creen.
  void addEventListener(WindowEventCallback callback);
  void removeEventListener(WindowEventCallback callback);
  void clearEventListener();

  Size get titleBarButtonSize;
  double get titleBarHeight;
  double get borderSize;
  bool get isMaximized;
}
