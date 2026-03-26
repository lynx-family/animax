// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.loader;

import static org.junit.Assert.*;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

public class AnimaXUnzipLoaderTest {
  private static final String Input_Text_File_Path = "input.txt";
  private static final String Zipped_File_Path = "zipped.zip";
  private static final String Unzipped_File_Path = "unzipped";

  // Method to write content to a file using a File object
  private static void writeFile(File file, String content) throws IOException {
    try (BufferedWriter writer = new BufferedWriter(new FileWriter(file))) {
      writer.write(content);
      System.out.println("File created and content written: " + file.getAbsolutePath());
    }
  }

  // Method to zip a file into a ZIP file using File objects
  private static void zipFile(File file, File zipFile) throws IOException {
    try (FileInputStream fis = new FileInputStream(file);
         FileOutputStream fos = new FileOutputStream(zipFile);
         ZipOutputStream zos = new ZipOutputStream(fos)) {
      // Create a new ZipEntry for the file
      ZipEntry zipEntry = new ZipEntry(file.getName());
      zos.putNextEntry(zipEntry);

      // Write the contents of the file to the ZIP file
      byte[] buffer = new byte[1024];
      int len;
      while ((len = fis.read(buffer)) > 0) {
        zos.write(buffer, 0, len);
      }

      // Close the entry
      zos.closeEntry();
      System.out.println("File zipped successfully: " + zipFile.getAbsolutePath());
    }
  }

  // Method to compare the contents of two text files
  public static boolean compareTextFiles(File file1, File file2) {
    try (BufferedReader reader1 = new BufferedReader(new FileReader(file1));
         BufferedReader reader2 = new BufferedReader(new FileReader(file2))) {
      String line1 = reader1.readLine();
      String line2 = reader2.readLine();

      while (line1 != null || line2 != null) {
        if (line1 == null || line2 == null) {
          // One file has more lines than the other
          return false;
        } else if (!line1.equals(line2)) {
          // Lines are different
          return false;
        }

        // Read the next line from both files
        line1 = reader1.readLine();
        line2 = reader2.readLine();
      }

      // If we reach here, the files are identical
      return true;
    } catch (IOException e) {
      return false;
    }
  }

  @Test
  public void testUnzipWithInvalidArguments() {
    String zippedFilePath = "some_file_path";
    AnimaXUnzipLoader.UnzipResult res = AnimaXUnzipLoader.unzip(zippedFilePath, null);

    assertNull(res.getPath());
    assertNotNull(res.getError());

    String unzippedFilePath = "some_file_path";
    AnimaXUnzipLoader.UnzipResult res2 = AnimaXUnzipLoader.unzip(null, unzippedFilePath);

    assertNull(res2.getPath());
    assertNotNull(res2.getError());
  }

  // Method to print the content of a binary file
  public static void printBinaryFileContent(File file) throws IOException {
    try (FileInputStream fis = new FileInputStream(file)) {
      int byteData;
      while ((byteData = fis.read()) != -1) {
        System.out.print(byteData + " ");
      }
    }
  }

  @Test
  public void testUnzipWithValidArguments() throws IOException {
    // Declare File variables
    File inputFile = null;
    File zippedFile = null;
    File unzippedDir = null;

    TemporaryFolder tempFolder = new TemporaryFolder();
    tempFolder.create();

    try {
      inputFile = tempFolder.newFile(Input_Text_File_Path);
      zippedFile = tempFolder.newFile(Zipped_File_Path);
      unzippedDir = tempFolder.newFolder(Unzipped_File_Path);
    } catch (IOException e) {
      fail("Files cannot be created.");
    }

    try {
      writeFile(inputFile,
          "It was a bright cold day in April, and the clocks were striking thirteen. Winston Smith, his chin nuzzled into his breast in an effort to escape the vile wind, slipped quickly through the glass doors of Victory Mansions, though not quickly enough to prevent a swirl of gritty dust from entering along with him.");
    } catch (IOException e) {
      fail("Failed to write to the file: " + e.getMessage());
    }

    // Zip the inputFile into zippedFile
    try {
      zipFile(inputFile, zippedFile);
    } catch (IOException e) {
      fail("Failed to zip file: " + e.getMessage());
    }

    AnimaXUnzipLoader.UnzipResult res =
        AnimaXUnzipLoader.unzip(zippedFile.getAbsolutePath(), unzippedDir.getAbsolutePath());

    System.out.println(res.getPath());
    System.out.println(res.getError());

    assertNotNull(res.getPath());
    assertNull(res.getError());

    File unzippedFile = new File(res.getPath() + '/' + Input_Text_File_Path);

    assertTrue(compareTextFiles(inputFile, unzippedFile));

    tempFolder.delete();
  }
}
