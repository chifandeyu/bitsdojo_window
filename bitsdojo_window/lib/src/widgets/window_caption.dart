import 'package:flutter/widgets.dart';
import '../app_window.dart';
import 'package:flutter/foundation.dart' show kIsWeb;
import 'dart:io' show Platform;

class _MoveWindow extends StatelessWidget {
  _MoveWindow({Key? key, this.child, this.onDoubleTap}) : super(key: key);
  final Widget? child;
  final VoidCallback? onDoubleTap;
  @override
  Widget build(BuildContext context) {
    return GestureDetector(
        behavior: HitTestBehavior.translucent,
        onPanStart: (details) {
          appWindow.startDragging();
        },
        onDoubleTap: this.onDoubleTap ?? () => appWindow.maximizeOrRestore(),
        child: this.child ?? Container());
  }
}

class MoveWindow extends StatelessWidget {
  final Widget? child;
  final VoidCallback? onDoubleTap;
  MoveWindow({Key? key, this.child, this.onDoubleTap}) : super(key: key);
  @override
  Widget build(BuildContext context) {
    if (child == null) return _MoveWindow(onDoubleTap: this.onDoubleTap);
    final childWidget = Platform.isMacOS ? Expanded(child: this.child!) : Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [Expanded(child: this.child!)]);
    return _MoveWindow(
      onDoubleTap: this.onDoubleTap,
      child: childWidget,
    );
  }
}

class WindowTitleBarBox extends StatelessWidget {
  final Widget? child;
  WindowTitleBarBox({Key? key, this.child}) : super(key: key);
  @override
  Widget build(BuildContext context) {
    if (kIsWeb) {
      return Container();
    }
    final titlebarHeight = appWindow.titleBarHeight;
    return SizedBox(height: titlebarHeight, child: this.child ?? Container());
  }
}
