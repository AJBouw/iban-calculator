package nl.iban.e2e;

import org.junit.platform.suite.api.IncludeEngines;
import org.junit.platform.suite.api.SelectPackages;
import org.junit.platform.suite.api.Suite;

/**
 * Entry point for Maven Surefire: runs every .feature file in the package
 * "features" (src/test/resources/features) with the Cucumber engine. Glue code
 * and report settings are in src/test/resources/junit-platform.properties.
 */
@Suite
@IncludeEngines("cucumber")
@SelectPackages("features")
public class RunCucumberTest {
}
