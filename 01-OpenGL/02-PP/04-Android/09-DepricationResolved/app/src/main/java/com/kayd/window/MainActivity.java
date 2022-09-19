package com.kayd.window;

// default packages
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;

// for Color.BLACK
import android.graphics.Color;
import android.content.pm.ActivityInfo;  // for ActivityInfo

// for fullscreen
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import androidx.core.view.WindowInsetsCompat;

public class MainActivity extends AppCompatActivity {
    private MyView view;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        /* prepare to enable fullscreen */
        // hide action bar
        getSupportActionBar().hide();

        // disallow Decor from fitting system windows anywhere when this app is run
        WindowCompat.setDecorFitsSystemWindows(getWindow(), false);

        // hiding insets, i.e. system bars and IME (Input Method Editor, a.k.a. Keyboard)
        WindowInsetsControllerCompat windowInsetsController = WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());
        windowInsetsController.hide(
            WindowInsetsCompat.Type.systemBars() |
            WindowInsetsCompat.Type.ime()
        );

        // forced landscape orientation
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        getWindow().getDecorView().setBackgroundColor(Color.BLACK);
        
        view = new MyView(this);
        setContentView(view);
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }
}