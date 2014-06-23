package de._4z2.msc

import scala.collection.mutable.ArrayBuffer

object App {

  def main(args : Array[String]) {
    val root = new Node[Int](0)
    val tree = new Tree[Node[Int]](root)
    (1 to 3).foreach(i => root.newChild(i))
    (1 to 2).foreach(i => root.children(0).newChild(i+3))
    root.children(2).newChild(6)

    println(tree)
    root.children(0).children(1).mergeWithLeftSibling(7)
    println(tree)
    root.children(0).mergeWithRightSibling(8)
    println(tree)
    root.children(1).mergeWithOnlyChild(9)
    println(root)
    root.children(0).mergeWithRightSibling(10)
    println(root)
    root.children(0).mergeWithOnlyChild(11)
    println(root)
  }

}
