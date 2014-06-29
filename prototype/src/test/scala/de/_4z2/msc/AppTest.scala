package de._4z2.msc
import org.scalatest.FunSuite
import org.scalatest.FeatureSpec

 class AppTest extends FunSuite {

  test("my first test") {
    assertResult("hello") {
      "hello"
    }
  }
}

