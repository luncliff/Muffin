// see https://docs.gradle.org/current/javadoc/index.html
pluginManagement {
    repositories {
        gradlePluginPortal()
        google()
        mavenCentral()
    }
}
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.PREFER_PROJECT)
    repositories {
        google()
        mavenCentral()
    }
}
// rootProject.name = "Muffin"
include(":muffin")
include(":sample")
