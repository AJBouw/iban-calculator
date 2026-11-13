package nl.iban.e2e;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.junit.jupiter.api.Assertions.fail;

import io.cucumber.java.en.Given;
import io.cucumber.java.en.Then;
import io.cucumber.java.en.When;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;

/**
 * Step definitions: they start the compiled program as a separate process,
 * write the scenario's input to its standard input and keep its standard
 * output, standard error and exit code for the Then steps.
 *
 * <p>Cucumber creates a new instance for every scenario, so the fields never
 * carry state from one scenario to the next.
 */
public class IbanSteps {

    /** A scenario fails instead of hanging when the program does not finish. */
    private static final long TIMEOUT_SECONDS = 30;

    private String input = "";
    private String output = "";
    private String error = "";
    private int exitCode = -1;

    @Given("the input {string}")
    public void theInput(String line) {
        input = line + "\n";
    }

    /** One value per line, as a user types them after each question. */
    @Given("the input lines:")
    public void theInputLines(String lines) {
        input = lines.endsWith("\n") ? lines : lines + "\n";
    }

    @Given("no input")
    public void noInput() {
        input = "";
    }

    @When("the program runs")
    public void theProgramRuns() throws IOException, InterruptedException {
        Process process = new ProcessBuilder(executable()).start();

        // Standard output and standard error are read at the same time: a
        // program that fills one pipe while the test waits on the other would
        // otherwise block forever.
        CompletableFuture<String> stdout = readAsync(process.getInputStream());
        CompletableFuture<String> stderr = readAsync(process.getErrorStream());

        try (OutputStream stdin = process.getOutputStream()) {
            stdin.write(input.getBytes(StandardCharsets.UTF_8));
        }
        // Closing standard input signals end of input, as Ctrl+D does in a terminal.

        if (!process.waitFor(TIMEOUT_SECONDS, TimeUnit.SECONDS)) {
            process.destroyForcibly();
            fail("the program did not finish within " + TIMEOUT_SECONDS + " seconds");
        }
        exitCode = process.exitValue();
        output = stdout.join();
        error = stderr.join();
    }

    @Then("the output is {string}")
    public void theOutputIs(String expected) {
        // Line endings differ between platforms (\r\n on Windows), so they are
        // normalised before comparing.
        assertEquals(expected, normalise(output).strip(), "standard output");
    }

    /**
     * Count mode prints only the number of valid account numbers in [b, e).
     * Same check as "the output is", worded in the language of the assignment.
     */
    // LEARN: (s) in a Cucumber expression is optional text: "1 valid account number" also matches.
    @Then("the program counts {int} valid account number(s)")
    public void theProgramCounts(int expected) {
        assertEquals(String.valueOf(expected), normalise(output).strip(),
                "number of valid account numbers on standard output");
    }

    @Then("the output is:")
    public void theOutputIsBlock(String expected) {
        assertEquals(normalise(expected).strip(), normalise(output).strip(), "standard output");
    }

    @Then("the output is empty")
    public void theOutputIsEmpty() {
        assertEquals("", normalise(output).strip(), "standard output");
    }

    @Then("the program succeeds")
    public void theProgramSucceeds() {
        assertEquals(0, exitCode, "exit code; standard error was: " + error);
    }

    @Then("the program fails")
    public void theProgramFails() {
        assertNotEquals(0, exitCode, "exit code; standard output was: " + output);
    }

    @Then("the error message contains {string}")
    public void theErrorMessageContains(String fragment) {
        assertTrue(error.contains(fragment),
                "standard error should contain \"" + fragment + "\" but was: " + error);
    }

    private static String executable() {
        String path = System.getProperty("iban.executable", "");
        if (path.isBlank()) {
            fail("set -Diban.executable=<path to the iban program> (CTest does this automatically)");
        }
        return path;
    }

    private static CompletableFuture<String> readAsync(InputStream stream) {
        return CompletableFuture.supplyAsync(() -> {
            try (stream) {
                return new String(stream.readAllBytes(), StandardCharsets.UTF_8);
            } catch (IOException exception) {
                throw new java.io.UncheckedIOException(exception);
            }
        });
    }

    private static String normalise(String text) {
        return text.replace("\r\n", "\n");
    }
}