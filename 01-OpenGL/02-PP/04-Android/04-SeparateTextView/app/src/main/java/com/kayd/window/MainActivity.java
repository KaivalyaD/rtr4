package com.kayd.window;

// default packages
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;

// for Color.BLACK
import android.graphics.Color;

public class MainActivity extends AppCompatActivity {
    private View view;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().getDecorView().setBackgroundColor(Color.BLACK);
        
        view = new View(this);
        setContentView(view);
    }
}