import Cocoa
import FlutterMacOS

public class BitsdojoWindowPlugin: NSObject, FlutterPlugin {
  public static func register(with registrar: FlutterPluginRegistrar) {
    let channel = FlutterMethodChannel(name: "bitsdojo/window", binaryMessenger: registrar.messenger)
    let instance = BitsdojoWindowPlugin(channel)
    registrar.addMethodCallDelegate(instance, channel: channel)
  }

  private let channel: FlutterMethodChannel

  public init(_ channel: FlutterMethodChannel) {
    self.channel = channel
    super.init()
    NotificationCenter.default.addObserver(self, selector: #selector(onWillEnterFullScreen),
                                           name: NSWindow.willEnterFullScreenNotification, object: nil)
    NotificationCenter.default.addObserver(self, selector: #selector(onWillExitFullScreen),
                                           name: NSWindow.willExitFullScreenNotification, object: nil)
  }

  @objc func onWillEnterFullScreen(notification: Notification) {
    _EmitEvent(event: "enter-full-screen")
  }

  @objc func onWillExitFullScreen(notification: Notification) {
    _EmitEvent(event: "leave-full-screen")
  }

  private func _EmitEvent(event: String) {
    channel.invokeMethod("onEvent", arguments: ["eventName": event])
  }

  public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {    
    switch call.method {
    /*
     // TODO: implement this for channel methods
     case "getAppWindow":
        getAppWindow(call, result)
    */
    default:
      result(FlutterMethodNotImplemented)
    }
  }
}
