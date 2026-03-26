package com.lynx.animax;

import static com.lynx.animax.util.StreamUtil.clamp;
import static com.lynx.animax.util.StreamUtil.createFileInputStream;
import static com.lynx.animax.util.StreamUtil.getByteArrayFromInputStream;
import static com.lynx.animax.util.StreamUtil.saveFileFromInputStream;
import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import android.content.Context;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;

@RunWith(AndroidJUnit4.class)
public class StreamUtilTest {
  @Test
  public void testClampValueWithinRange() {
    assertEquals(5, clamp(5, 1, 10));
  }

  @Test
  public void testClampValueBelowRange() {
    assertEquals(1, clamp(0, 1, 10));
  }

  @Test
  public void testClampValueAboveRange() {
    assertEquals(10, clamp(11, 1, 10));
  }

  @Test
  public void testGetByteArrayFromInputStream_NoHint_ValidInput() {
    byte[] expectedBytes = "test data".getBytes();
    ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(expectedBytes);
    byte[] result = getByteArrayFromInputStream(byteArrayInputStream);
    assertArrayEquals("The byte array should match the input stream's data", expectedBytes, result);
  }

  @Test
  public void testGetByteArrayFromInputStream_ValidInput() throws Exception {
    byte[] expectedBytes = "test data".getBytes();
    ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(expectedBytes);

    byte[] result = getByteArrayFromInputStream(byteArrayInputStream, expectedBytes.length);
    assertArrayEquals("The byte array should match the input stream's data", expectedBytes, result);
  }

  @Test
  public void testGetByteArrayFromInputStream_InputStream_read_IOException() throws Exception {
    InputStream mockInputStream = mock(InputStream.class);
    when(mockInputStream.read(any(byte[].class), anyInt(), anyInt()))
        .thenThrow(new IOException("Forced IOException"));
    assertNull("Method should return null if an IOException occurs",
        getByteArrayFromInputStream(mockInputStream, 100));
  }

  @Test
  public void testGetByteArrayFromInputStream_InputStream_close_IOException() throws Exception {
    InputStream mockInputStream = mock(InputStream.class);
    when(mockInputStream.read(any(byte[].class), anyInt(), anyInt())).thenReturn(-1);
    Mockito.doThrow(new IOException("Forced IOException")).when(mockInputStream).close();
    assertNotNull("Method should not return null if an IOException occurs on close()",
        getByteArrayFromInputStream(mockInputStream, 100));
  }

  @Test
  public void testSaveFileFromInputStream_InputStream_null() {
    Context context = InstrumentationRegistry.getInstrumentation().getTargetContext();
    String testFileName = "testSaveFileFromInputStream_Success_tmpFile.txt";
    File testFile = new File(context.getFilesDir(), testFileName);
    assertFalse(saveFileFromInputStream(null, 100, testFile.getAbsolutePath()));
    testFile.delete();
  }

  @Test
  public void testSaveFileFromInputStream_InvalidFilePath() throws IOException {
    InputStream mockInputStream = mock(InputStream.class);
    when(mockInputStream.read(any(byte[].class), anyInt(), anyInt())).thenReturn(-1);
    assertFalse(saveFileFromInputStream(mockInputStream, 100, "invalid"));
  }

  @Test
  public void testSaveFileFromInputStream_Success() throws IOException {
    Context context = InstrumentationRegistry.getInstrumentation().getTargetContext();
    String testFileName = "testSaveFileFromInputStream_Success_tmpFile.txt";
    File testFile = new File(context.getFilesDir(), testFileName);

    String testData = "This is a test string.";
    ByteArrayInputStream inputStream = new ByteArrayInputStream(testData.getBytes());
    int totalLength = testData.getBytes().length;
    boolean result = saveFileFromInputStream(inputStream, totalLength, testFile.getAbsolutePath());
    assertTrue("File should be saved successfully", result);

    byte[] fileContents = new byte[totalLength];
    try (FileInputStream fileInputStream = new FileInputStream(testFile)) {
      int readLength = fileInputStream.read(fileContents);
      assertEquals("Read length should match written length", totalLength, readLength);
      assertArrayEquals("File contents should match input data", testData.getBytes(), fileContents);
    }

    testFile.delete();
  }

  @Test
  public void testSaveFileFromInputStream_NoHint_Success() throws IOException {
    Context context = InstrumentationRegistry.getInstrumentation().getTargetContext();
    String testFileName = "testSaveFileFromInputStream_Success_tmpFile.txt";
    File testFile = new File(context.getFilesDir(), testFileName);

    String testData = "This is a test string.";
    ByteArrayInputStream inputStream = new ByteArrayInputStream(testData.getBytes());
    int totalLength = testData.getBytes().length;
    boolean result = saveFileFromInputStream(inputStream, testFile.getAbsolutePath());
    assertTrue("File should be saved successfully", result);

    byte[] fileContents = new byte[totalLength];
    try (FileInputStream fileInputStream = new FileInputStream(testFile)) {
      int readLength = fileInputStream.read(fileContents);
      assertEquals("Read length should match written length", totalLength, readLength);
      assertArrayEquals("File contents should match input data", testData.getBytes(), fileContents);
    }

    testFile.delete();
  }

  @Test
  public void testCreateFileInputStream_path_null() {
    assertNull(createFileInputStream(null));
  }

  @Test
  public void testCreateFileInputStream_path_empty() {
    assertNull(createFileInputStream(""));
  }

  @Test
  public void testCreateFileInputStream_path_invalid() {
    assertNull(createFileInputStream("InvalidPath"));
  }

  @Test
  public void testCreateFileInputStream_path_exist() throws IOException {
    Context context = InstrumentationRegistry.getInstrumentation().getTargetContext();
    String testFileName = "testSaveFileFromInputStream_Success_tmpFile.txt";
    File testFile = new File(context.getFilesDir(), testFileName);
    assertTrue(testFile.createNewFile());
    FileInputStream inputStream = createFileInputStream(testFile.getAbsolutePath());
    assertNotNull(inputStream);
    inputStream.close();
    assertTrue(testFile.delete());
  }

  @Test
  public void testCreateFileInputStream_directory_fail() throws IOException {
    Context context = InstrumentationRegistry.getInstrumentation().getTargetContext();
    String testDirName = "testCreateFileInputStream_directory_fail_testDirectory_tmp";
    File testDir = new File(context.getFilesDir(), testDirName);
    testDir.mkdir();
    try {
      // Attempt to create a FileInputStream with a directory path (should fail)
      FileInputStream inputStream = createFileInputStream(testDir.getAbsolutePath());
      assertNull("InputStream should be null for a directory path", inputStream);
    } finally {
      // Clean up: delete the directory
      assertTrue("Directory should be deleted", testDir.delete());
    }
  }
}
