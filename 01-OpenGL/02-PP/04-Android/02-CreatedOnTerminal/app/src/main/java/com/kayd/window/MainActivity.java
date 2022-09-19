package com.kayd.window;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;  // provides access to environment variables

/**
 * MainActivity IS the main class (registered in the manifest),
 * The loader constructs its object.
 * 
 * onCreate() is called when a process is created. Infact, there are 6
 * more process states represented by functions like:
 * onResume(), onStart(), onRestart(), onStop(), onPause() and onDestroy().
 */
public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        // this is partial overriding
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }
}
