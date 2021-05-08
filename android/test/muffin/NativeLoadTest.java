package muffin;

import org.junit.Before;
import org.junit.jupiter.api.Test;

public class NativeLoadTest {
    @Before
    public void checkSTL(){
        System.loadLibrary("c++_shared");
    }
    @Test
    public void check_muffin() {
        System.loadLibrary("muffin");
    }
}
