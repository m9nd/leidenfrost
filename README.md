# leidenfrost

local privilege escalation on windows by exploiting **CMSTP** to circumvent **User Access Control (UAC)**

# how it works

the leidenfrost attack involves deploying a specially crafted .ini file and utilizing cmstp.exe to install it. this process triggers the execution of a post-installation executable with elevated administrator privileges, thereby bypassing UAC safeguards! real fun stuff!

# changes

### i made the following changes to the deploy::craftPayload function:
    * the szExecutablePath parameter is now passed as a const reference to avoid unnecessary copying.
    * the randName variable is now declared as a const to prevent accidental modification.
    * the payload is constructed using a std::stringstream instead of concatenating strings.
    * the payloadStream.str() method is used to return the final string instead of declaring a new std::string and copying the contents of the stream into it.

###  i made the following changes to the deploy::writePayload function:
    * function name: the function name has been modified from write_payload to writePayload to follow the camel case naming convention.
    * exception handling: error handling has been added using try-catch blocks. two specific exceptions are caught and handled: std::filesystem::filesystem_error for handling errors during file removal, and      std::ofstream::failure for handling errors during file writing.
    * file stream declaration: instead of declaring std::ofstream separately and later opening the file using the open function, the file stream is now declared and opened in a single step.
    * namespace qualification: the function craft_payload is now called using the deploy:: qualification to specify the namespace.
    * parameter type: the parameter szExecutablePath has been changed to a const std::string& reference to avoid unnecessary copying of the argument.

### i made the following changes to the deploy::runPayload function
    * function name: the function name has been modified from run_payload to runPayload to follow the camel case naming convention.
    * variable names: the variable names iniFileName and executableName have been changed to iniFile and executableName.
    * namespace qualification: the functions kill_by_name and create_process have been changed to killByName and createProcess to follow the camel case naming convention.
    * parameter type: the function createProcess now accepts a single string argument instead of concatenating the string inside the function call.

### i made the following changes to the deploy::cleanup function
    * error handling: the updated code includes error handling for the file removal operation. it checks if the file exists using std::filesystem::exists and then attempts to remove it using std::filesystem::remove. if an error occurs during the removal, an error code ec is set, but you would need to manually implement handling or reporting of the error to the code. 
    * return value: the updated code explicitly returns true at the end of the function, regardless of whether the file removal operation was successful or not.

### i made the following changes to the deploy::craftPayload function
    * function name: the function name craft_payload has been changed to craftPayload to follow the camel case naming convention.
    * variable names: the variable randName has been changed to randName to follow the camel case naming convention.
    * utils::b64_decode: the original code uses utils::b64_decode to decode the base64 payload parts. i modified it to use utils::base64Decode instead.
    * std::string reserve: the improved code uses payload.reserve to preallocate memory for the payload string based on the sizes of the parts and additional characters, improving performance by avoiding unnecessary memory reallocations.
    * string concatenation: instead of using += for string concatenation, the improved code uses + to concatenate the different parts and literals. 
    * fixed backslash in "\ShortSvcName\": the original code had an escape sequence "\S" that was fixed in the improved code to "\ShortSvcName\".

### i made the following changes to the randomString function 
    * random number generation: te original code uses srand(__rdtsc()) to seed the random number generator, which relies on the timestamp std::random_device is now used to obtain a random seed, and then std::mt19937 engine is used as the random number engine. thus ensuring a robust and reliable random number generation! extremely exciting improvements!
    * character set definition: the character set is now defined as a const std::string literal directly, whereas in the original code by our lovely xerosic, is defined as a character array (const char charset[]). 
    * random character selection: in the modified code, a std::uniform_int_distribution<size_t> is used to generate random indices within the range of the character set. these indices are used to select random characters from the character set. this approach ensures a uniform distribution of random characters.
    * string initialization: instead of initializing the string str with a value of 0 (std::string str(nLength, 0) in the original code), we now initialize it with a space character (std::string str(nLength, ' ')). thus ensuring that the resulting string doesn't contain null characters, which could lead to some unwanted tomfoolery

### i made the following changes to the base64Decode function
    * function name: the function name has been changed from utils::b64_decode to utils::base64Decode because it looks cleaner and it follows the camel case naming convention!
    * variable name: the variable szBuffer has been renamed to base64String.

# credits
[xerosic](https://github.com/xerosic/leidenfrost) - for 90% of the code in this project and being sexy

[ferret](https://github.com/ferrislovescpp/leidenfrost/) - for 10% of the code and majority comments 
