# libp Testing

This is a general guide on how to write tests for lib. All tests should live in the `test/` director and follow the naming convention `<module>_test.c`(eg. `pstring_test.c`). Each test file targets a single module and must `#include "testlib.h` to integrate with the test suite.

## File Structure

The basic layout of a test includes:

- **Setup:** Optional global variables or a `setup()` function to initialize test_state.
- **Test Functions:** Each function tests one behavior and follows the arrange-act-assert pattern.
- **Main Function:** Calls `test_handler` for each test, followed by `test_output(__FILE__)` to display results.

[!Note]
`setup()` does not automatically run before tests, you will need to call it during the arrange block. You can use what ever name you wish for test set up functions_

All test functions must:

- Be named descriptively and clearly indicate the behavior under test.
- Contain no parameters and return void.
- Be registered in `main()` using `test_handler("description", func_name)`

### Example Test

```c
// example_test.c
#include "example.c"
#include "testlib.h"

// Test Variables
static char *empty_string = "test string";
static int num = 69;

// Test Helper Functions
void setup() {
    // some setup stuff
}

// Example Tests
void foo_non_empty_string() {
    setup();
    
    char* result = foo(num);

    assert_continue(result)
}

// Main function
int main() {
    test_handler("foo returns non empty string", foo_non_empty_string);
    test_output(__FILE__);
}
```

## Test Structure

Most tests will have the same structure. This structure can be referred to as **arrange**, **act**, **assert**. Tests must _arrange_ the state of the program, _act_ on the function under test, then _assert_ that the program is in the expected state afterwards.

These steps consist of the following:

- The **arrange** step is for setting up details of the program specific to the test. This involves created variables and instantiating structs.

- The **act** step if where you actually make the calls to the function or feature you are testing. This will usually be a single function call but if multiple are being tested they will all be in this block

- The **assert** step is where you make assertions about the program state after the act step. Here you ensure that your function and features are working as intended by verification.

Here is an example of good and bad test functions

```c
// my_tests.c

void fib(){
    // fibonacci sequence function
}

void bad_test_function() {
    int expected_value_1 = 55
    int result_1 = fib(10) // Separate blocks from each other
    assert_continue(result_1 == expected_value_1)

    
    // Don't intertwine steps
    int expected_value_2 = 69;
    int result_2 = some_other_function();
    // Only 1 behavior should be tested per function
    assert_continue(result_2 + 17 * 6, expected_value_1 + expected_value_2)
    // Keep assertions simple
}

void good_test_function() {
    // Arrange
    int result;
    int expected_value = 610;

    // Act
    result = fib(15)

    // Assert
    assert_continue(result == expected_value);
}

```

[!Note]
If needed after the assert block another section for cleaning up arrange (freeing any memory, closing files, etc) can be done. This can also be encapsulated in a `clean_up()` function.
