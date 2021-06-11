package muffin;

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
    final boolean finished = resume(handle);
    if (finished)
      handle = 0;
  }
  private native boolean resume(long handle);
}
