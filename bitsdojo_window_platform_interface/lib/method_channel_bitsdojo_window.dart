import 'package:flutter/services.dart';

import 'bitsdojo_window_platform_interface.dart';

const MethodChannel _channel = MethodChannel('bitsdojo/window');

const String _enterFullScreen = 'enter-full-screen';
const String _leaveFullScreen = 'leave-full-screen';
const String _hideCursor = 'hide-cursor';
const String _showCursor = 'show-cursor';

/// An implementation of [BitsdojoWindowPlatform] that uses method channels.
class MethodChannelBitsdojoWindow extends BitsdojoWindowPlatform {
  MethodChannelBitsdojoWindow() {
    _channel.setMethodCallHandler(_methodCallHandler);
  }

  final Set<WindowEventCallback?> _callbackList = {};

  @override
  void dragAppWindow() async {
    try {
      await _channel.invokeMethod('dragAppWindow');
    } catch (e) {
      print("Could not start draggging -> $e");
    }
  }

  @override
  void addEventListener(WindowEventCallback callback) {
    _callbackList.add(callback);
  }

  @override
  void removeEventListener(WindowEventCallback callback) {
    _callbackList.remove(callback);
  }

  @override
  void clearEventListener() {
    _callbackList.clear();
  }

  Future<void> _methodCallHandler(MethodCall call) async {
    if (call.method != 'onEvent') throw UnimplementedError();
    final eventName = call.arguments['eventName'];
    //print('==== BitsdojoWindow eventName: ${eventName}');
    WindowEventType eventType = WindowEventType.unknownType;
    if (eventName == _enterFullScreen) {
      eventType = WindowEventType.enterFullScreen;
    } else if (eventName == _leaveFullScreen) {
      eventType = WindowEventType.leaveFullScreen;
    } else if (eventName == _hideCursor) {
      eventType = WindowEventType.hideCursor;
    } else if (eventName == _showCursor) {
      eventType = WindowEventType.showCursor;
    } else {
      return;
    }
    _callbackList.forEach((element) {
      element?.call(eventType);
    });
  }
}
