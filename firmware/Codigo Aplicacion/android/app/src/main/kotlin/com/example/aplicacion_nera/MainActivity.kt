package com.example.aplicacion_nera

import io.flutter.embedding.android.FlutterActivity
import io.flutter.embedding.engine.FlutterEngine
import io.flutter.plugin.common.MethodChannel
import android.os.Build
import android.util.AtomicFile
import java.io.File
import java.util.concurrent.Executors

class MainActivity : FlutterActivity() {
    private val storageWriter = Executors.newSingleThreadExecutor()
    private val historyFile by lazy { AtomicFile(File(filesDir, "nera_history_v1.json")) }
    override fun configureFlutterEngine(flutterEngine: FlutterEngine) {
        super.configureFlutterEngine(flutterEngine)
        MethodChannel(flutterEngine.dartExecutor.binaryMessenger, "nera/platform")
            .setMethodCallHandler { call, result ->
                when (call.method) {
                    "androidSdk" -> result.success(Build.VERSION.SDK_INT)
                    "loadUserPreferences" -> result.success(
                        getSharedPreferences("nera_user", MODE_PRIVATE).getString("preferences_v1", null))
                    "saveUserPreferences" -> {
                        val text = call.arguments as? String
                        if (text == null) result.error("invalid", "Preferencias invalidas", null)
                        else storageWriter.execute {
                            // FUNCIONAL: confirmar solo despues de escribir, fuera del hilo UI.
                            try {
                                val saved = getSharedPreferences("nera_user", MODE_PRIVATE)
                                    .edit().putString("preferences_v1", text).commit()
                                runOnUiThread { result.success(saved) }
                            } catch (error: Exception) {
                                runOnUiThread { result.error("save_failed", error.message, null) }
                            }
                        }
                    }
                    "loadHistory" -> storageWriter.execute {
                        try {
                            val text = if (historyFile.baseFile.exists())
                                String(historyFile.readFully(), Charsets.UTF_8) else null
                            runOnUiThread { result.success(text) }
                        } catch (error: Exception) {
                            runOnUiThread { result.error("load_history_failed", error.message, null) }
                        }
                    }
                    "saveHistory" -> {
                        val text = call.arguments as? String
                        if (text == null) result.error("invalid", "Historial invalido", null)
                        else storageWriter.execute {
                            var stream: java.io.FileOutputStream? = null
                            try {
                                stream = historyFile.startWrite()
                                stream.write(text.toByteArray(Charsets.UTF_8))
                                historyFile.finishWrite(stream)
                                runOnUiThread { result.success(true) }
                            } catch (error: Exception) {
                                if (stream != null) historyFile.failWrite(stream)
                                runOnUiThread { result.error("save_history_failed", error.message, null) }
                            }
                        }
                    }
                    else -> result.notImplemented()
                }
            }
    }
    override fun onDestroy() {
        storageWriter.shutdown()
        super.onDestroy()
    }
}
