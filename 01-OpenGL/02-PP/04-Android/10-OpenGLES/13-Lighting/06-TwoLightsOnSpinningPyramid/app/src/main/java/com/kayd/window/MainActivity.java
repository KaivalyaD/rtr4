package com.kayd.window;

// default packages
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;

import android.graphics.Color;  // for Color.BLACK
import android.content.pm.ActivityInfo;  // for ActivityInfo

// for fullscreen
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import androidx.core.view.WindowInsetsCompat;

public class MainActivity extends AppCompatActivity {
    private GLESView glesView;

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

        /* force landscape orientation */
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        /* set background to pitch black by default */
        getWindow().getDecorView().setBackgroundColor(Color.BLACK);
        
        /* set view to our custom GLES-powered view */
        glesView = new GLESView(this);
        setContentView(glesView);
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
