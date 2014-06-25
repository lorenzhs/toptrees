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
    println()

    val t = new OrderedTree[TreeNode, TreeEdge](() => new TreeNode(), () => new TreeEdge())
    t.addNodes(9)
    List(0->1, 0->2, 0->3, 1->4, 1->5, 3->6, 6 -> 7, 7 -> 8).foreach(p => t.addEdge(p._1, p._2))
    println(t)

    while(t.numEdges > 1) {
        println("Horz Merges")
        t.horizontalMerges
        println("Vert Merges")
        t.verticalMerges
        println(t)
        println()
    }
/*
    t.mergeNodeWithSibling(4,5)
    t.mergeNodeWithSibling(1,2)
    t.mergeNodeWithOnlyChild(3)
    t.mergeNodeWithSibling(1,3);
    t.mergeNodeWithOnlyChild(1);
*/
  }

}
