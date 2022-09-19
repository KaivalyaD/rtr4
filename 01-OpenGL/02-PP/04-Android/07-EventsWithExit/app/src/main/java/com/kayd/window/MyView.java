package com.kayd.window;

import androidx.appcompat.widget.AppCompatTextView;  // for AppCompatTextView

import android.content.Context;  // for Context
import android.graphics.Color;  // for Color.rgb()
import android.view.Gravity;  // for Gravity.CENTER

// Event related packages
import android.view.GestureDetector.OnDoubleTapListener;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector;
import android.view.MotionEvent;

public class MyView extends AppCompatTextView implements OnDoubleTapListener, OnGestureListener {
    private GestureDetector gestureDetector;

    public MyView(Context context) {
        super(context);

        setTextSize(48);
        setTextColor(Color.rgb(0, 255, 0));
        setGravity(Gravity.CENTER);
        setText("Hello, World!");

        gestureDetector = new GestureDetector(context, this, null, false);
        gestureDetector.setOnDoubleTapListener(this);
    }

    @Override
    public boolean onTouchEvent(MotionEvent e) {
        if(!gestureDetector.onTouchEvent(e))
            super.onTouchEvent(e);

        return true;
    }

    /* implementing OnDoubleTapListener */
    @Override
    public boolean onDoubleTap(MotionEvent e) {
        /*
         * this trigger shall toggle lighting
         */
        setText("Double tap detected");

        return true;
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e) {
        /* 
         * onDoubleTapEvent() may or may not carry forward to super,
         * but onDoublTap() always does; that's the reason we have
         * handled it and not this
         */

        return true;
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e) {
        /*
         * this trigger shall toggle animation
         */
        setText("Single tap detected");

        return true;
    }

    /* implementing OnGestureListener */
    @Override
    public boolean onDown(MotionEvent e) {
        
        return true;
    }

    @Override
    public boolean onFling(MotionEvent eIn, MotionEvent eOut, float velocityX, float velocityY) {
        
        return true;
    }

    @Override
    public void onLongPress(MotionEvent e) {
        /*
         * this trigger shall toggle textures
         */
        setText("Long press detected");
    }

    @Override
    public boolean onScroll(MotionEvent eIn, MotionEvent eOut, float distanceX, float distanceY) {
        System.exit(0);

        return true;
    }

    @Override
    public void onShowPress(MotionEvent e) {
    
    }

    @Override
    public boolean onSingleTapUp(MotionEvent e) {

        return true;
    }
}
