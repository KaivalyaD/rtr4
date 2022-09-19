package com.kayd.window;

// for AppCompatTextView
import androidx.appcompat.widget.AppCompatTextView;

// for Context
import android.content.Context;

// for Color.rgb()
import android.graphics.Color;

// for Gravity.CENTER
import android.view.Gravity;

public class MyView extends AppCompatTextView {

    public MyView(Context context) {
        super(context);

        setTextSize(48);
        setTextColor(Color.rgb(0, 255, 0));
        setGravity(Gravity.CENTER);
        setText("Hello, World!");
    }
}