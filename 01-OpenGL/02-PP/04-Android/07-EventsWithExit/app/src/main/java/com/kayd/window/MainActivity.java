package com.kayd.window;

// default packages
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;

// for Color.BLACK
import android.graphics.Color;
import android.view.View;  // for View
import android.view.Window;  // for Window
import android.content.pm.ActivityInfo;  // for ActivityInfo

public class MainActivity extends AppCompatActivity {
    private MyView view;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // forced landscape orientation
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        getWindow().getDecorView().setBackgroundColor(Color.BLACK);
        
        view = new MyView(this);
        setContentView(view);

        /* getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_FULLSCREEN |
            View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
            View.SYSTEM_UI_FLAG_IMMERSIVE
        ); */
        // supportRequestWindowFeature(Window.FEATURE_NO_TITLE);
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