import 'package:flutter/services.dart';
import 'watch_history.dart';

class WatchHistoryStore {
  static const _platform = MethodChannel('nera/platform');

  Future<WatchHistoryArchive> load() async {
    final text = await _platform.invokeMethod<String>('loadHistory');
    return text == null ? const WatchHistoryArchive([]) :
        WatchHistoryArchive.decode(text);
  }

  Future<void> save(WatchHistoryArchive archive) async {
    final saved = await _platform.invokeMethod<bool>('saveHistory', archive.encode());
    if (saved != true) throw PlatformException(code: 'save_history_failed');
  }
}
