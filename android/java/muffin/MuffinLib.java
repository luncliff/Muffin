package muffin;

/**
 * @author luncliff@gmail.com
 */
public class MuffinLib {
    static {
        System.loadLibrary("c++_shared");
        System.loadLibrary("muffin");
    }

}