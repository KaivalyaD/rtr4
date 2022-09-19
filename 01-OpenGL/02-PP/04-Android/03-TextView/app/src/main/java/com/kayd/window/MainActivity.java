package com.kayd.window;

// default packages
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;

// TextView related packages
import androidx.appcompat.widget.AppCompatTextView;

import android.graphics.Color;  // for Color.GREEN and Color.rgb()
import android.view.Gravity;  // for Gravity.CENTER

/**
 * MainActivity IS the main class (registered in the manifest),
 * The loader constructs its object.
 * 
 * onCreate() is called when a process is created. Infact, there are 7
 * more process states represented by functions like:
 * onResume(), onSuspend(), onStart(), onRestart(), onStop(), onPause() and onDestroy().
 */
public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        /* now there are no XML view layouts */
        // setContentView(R.layout.activity_main);

        getWindow().getDecorView().setBackgroundColor(Color.rgb(0, 0, 0));

        AppCompatTextView view = new AppCompatTextView(this);
        view.setTextSize(64);
        view.setTextColor(Color.GREEN);
        view.setGravity(Gravity.CENTER);
        view.setText("Hello, World!");

        setContentView(view);
    }
}