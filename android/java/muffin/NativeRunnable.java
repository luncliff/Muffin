package muffin;

import android.util.Log;

class NativeRunnable implements Runnable {
  static {
    System.loadLibrary("c++_shared");
    System.loadLibrary("muffin");
  }

  long handle;

  @Override
  public void run() {
    if (handle == 0)
      return;
    if (resume(handle) == true)
      return;
    handle = 0;
  }
  private native boolean resume(long handle);
}
