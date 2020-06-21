/**
 * @file filewriter_test.cpp
 */
#include "catch.hpp"
#include "mlboard.hpp"
#include <unistd.h>
#include <sstream>
#include <cstdio>
#include <sys/stat.h>
/**
 * Test the 2 filewriter objects at a time
 */
TEST_CASE("Writing two files at a time", "FileWritter")
{
    // Both files created at same time and hence there should be only one file
    mlboard::FileWriter f1,f2;
    mlboard::SummaryWriter<mlboard::FileWriter>::Scalar("Sample_1",1,1.1,f1);
    mlboard::SummaryWriter<mlboard::FileWriter>::Scalar("Sample_2",1,1.1,f2);
    REQUIRE(f1.FileName() == f2.FileName());
    remove(f1.FileName().c_str());
}

/**
 * Test the 2 filewriter objects at a different dir time
 */
TEST_CASE("Writing two files at a time in different paths", "FileWritter")
{
    // create temp dirs
    mkdir("_temp1_",0777);
    mkdir("_temp2_",0777);

    mlboard::FileWriter f1("_temp1_"),f2("_temp2_");
    mlboard::SummaryWriter<mlboard::FileWriter>::Scalar("Sample_1",1,1.1,f1);
    mlboard::SummaryWriter<mlboard::FileWriter>::Scalar("Sample_2",1,1.1,f2);
    REQUIRE(f1.FileName() != f2.FileName());

    REQUIRE(f1.FileName().substr(8,f1.FileName().length()) ==
        f2.FileName().substr(8,f2.FileName().length()));

    // remove temp dirs
    remove(f1.FileName().c_str());
    remove(f2.FileName().c_str());
    rmdir("_temp1_");
    rmdir("_temp2_");
}

/**
 * Test the summary writer
 */
TEST_CASE("Writing a summary to file", "FileWritter")
{
    // create temp dirs
    mkdir("_temp1_",0777);
    struct stat results;
    mlboard::FileWriter f1("_temp1_");

    for (int i = 1; i < 25; i++)
        mlboard::SummaryWriter<mlboard::FileWriter>::Scalar("Sample_1",i,1.1,f1);

    stat(f1.FileName().c_str(), &results);

    REQUIRE(results.st_size > 0);
    remove(f1.FileName().c_str());
    // remove temp dirs
    rmdir("_temp1_");
}
