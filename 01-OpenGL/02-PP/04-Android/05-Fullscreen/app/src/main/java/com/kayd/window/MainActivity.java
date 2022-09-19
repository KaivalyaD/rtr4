package com.kayd.window;

// default packages
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;

// for Color.BLACK
import android.graphics.Color;

// for View
import android.view.View;

// for Window
import android.view.Window;

public class MainActivity extends AppCompatActivity {
    private MyView view;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().getDecorView().setBackgroundColor(Color.BLACK);
        
        view = new MyView(this);
        setContentView(view);

        /*
         * for fullscreen:
         * 
         * 1. hide Title bar (embedded into the action bar)
         * 2. hide Navigation bar
         * 3. hide System UI
         */
        getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_FULLSCREEN |
            View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
            View.SYSTEM_UI_FLAG_IMMERSIVE
        );

        supportRequestWindowFeature(Window.FEATURE_NO_TITLE);
    }
}