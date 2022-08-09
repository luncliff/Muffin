package dev.luncliff.sample;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.graphics.PixelFormat;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.SurfaceHolder;
import android.view.Surface;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.TextView;
import dev.luncliff.AutoFitSurfaceView;
import dev.luncliff.sample.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity implements View.OnClickListener, SurfaceHolder.Callback2 {
    static {
        System.loadLibrary("sample");
    }
    static int windowFlags = WindowManager.LayoutParams.FLAG_FULLSCREEN
            | WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED;
    private ActivityMainBinding binding;
    private InputMethodManager inputs;
    private AutoFitSurfaceView view0;
    private SurfaceHolder holder0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(windowFlags, windowFlags);
        inputs = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        view0 = binding.sampleSurface;
        view0.setOnClickListener(this);
        holder0 = view0.getHolder();
        holder0.setFormat(PixelFormat.RGBX_8888);
        holder0.addCallback(this);
        view0.setAspectRatio(1920, 1080);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i("MainActivity", "onResume");
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.i("MainActivity", "onPause");
    }

    public native String stringFromJNI();

    public void hideSoftKeyboard(@NonNull View focused) {
        inputs.hideSoftInputFromWindow(focused.getWindowToken(), 0);
    }

    public void showSoftKeyboard(@NonNull View view) {
        if (view.requestFocus())
            inputs.showSoftInput(view, InputMethodManager.SHOW_IMPLICIT);
    }

    @Override
    public void onClick(View view) {
        Log.i("MainActivity", String.format("%s", stringFromJNI()));
    }

    @Override
    public void surfaceRedrawNeeded(@NonNull SurfaceHolder holder) {
        int rotation = view0.getDisplay().getRotation();
        Log.d("MainActivity", String.format("surfaceRedrawNeeded: %d", rotation));
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        Log.i("MainActivity", "surfaceCreated");
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        int rotation = view0.getDisplay().getRotation();
        Log.i("MainActivity", String.format("surfaceChanged format %d width %d height %d rotation %d", format, width,
                height, rotation));
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        Log.w("MainActivity", "surfaceDestroyed");
    }
}
