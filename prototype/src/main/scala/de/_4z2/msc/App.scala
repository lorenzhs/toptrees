package de._4z2.msc

import scala.collection.mutable.ArrayBuffer
import scala.collection.mutable.Map

object App {

  def main(args : Array[String]) {
/*
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
*/
    val filename = args.toList match {
        case List(x) => x
        case _ => "data/1998statistics.xml"
    }

    val start = System.nanoTime
    val (tree, map) = LazyXmlParser.parse(filename)
    val parsed = System.nanoTime
    println("Read input file in " + (parsed - start)/1000000.0 + "ms")

    val topTree = new TopTree(tree.numNodes)
    val nodeIds: Map[Int,Int] = (0 until tree.numNodes).map(i => (i, i))(scala.collection.breakOut)

    tree.doMerges((u, v, n, t) => {
        nodeIds(n) = topTree.addCluster(nodeIds(u), nodeIds(v), t)
    })

    println("done, " + (System.nanoTime - parsed)/1000000.0 + "ms")

/*
    val t = new OrderedTree[TreeNode, TreeEdge](() => new TreeNode(), () => new TreeEdge())
    t.addNodes(11)
    List(0->1, 0->2, 0->3, 1->4, 1->5, 3->6, 6 -> 7, 7 -> 8, 4 -> 9, 4 -> 10).foreach(p => t.addEdge(p._1, p._2))
    println(t)

    val tt = new TopTree(t.numNodes)
    println(tt)

    println("\n=== Doing all the merges now ===")
    //val nodeIds = (0 until t.numNodes).zipWithIndex.toMutableMap
    val nodeIds: Map[Int,Int] = (0 until t.numNodes).map(i => (i, i))(scala.collection.breakOut)
    println(nodeIds)
    t.doMerges((u, v, n, t) => {
        println("\tNodes " + u + " and " + v + " were merged into " + n + ", type=" + t)
        nodeIds(n) = tt.addCluster(nodeIds(u), nodeIds(v), t)
    })

    tt.clusters.zipWithIndex.filter(_._1.left >= 0).foreach(pair => println(pair._2 + ": " + pair._1))
    println(nodeIds)
*/
  }

}
