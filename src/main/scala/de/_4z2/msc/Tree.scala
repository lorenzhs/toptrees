package de._4z2.msc

import scala.collection.mutable.ArrayBuffer

class Tree[NodeType](val root: NodeType) {
  override def toString() = root.toString
}

class BinNode[DataType](val data: DataType, val parent: BinNode[DataType]) {
  var side = 0
  var left: Option[BinNode[DataType]] = None
  var right: Option[BinNode[DataType]] = None
  def this(_data: DataType) { this(_data, null) }  // root node constructor

  def left_=  (node: BinNode[DataType]):Unit = { node.side = -1; left = Some(node) }
  def right_= (node: BinNode[DataType]):Unit = { node.side = 1; right = Some(node) }

  def isLeaf = left == null && right == null
  def sibling = side match {
    case -1 => parent.right
    case  0 => None
    case  1 => parent.left
  }

  override def toString =
    "(" + List(left, Some(data), right).map(_.getOrElse("_").toString).mkString(",") + ")"
}

class Node(val parent: Node) {
  var index = -1
  val children = new ArrayBuffer[Node]()
  def this() { this(null) }

  def addChild(child: Node) { child.index = numChildren; children += child }
  def isLeaf = children.isEmpty
  def numChildren = children.size
  def hasSiblings = parent match {
    case null => false
    case node => node.children.size > 1
  }
  def isFirstChild = index == 0
  def isLastChild = index == parent.numChildren-1
  def leftSibling = if (isFirstChild) None else Some(parent.children(index - 1))
  def rightSibling = if (isLastChild) None else Some(parent.children(index + 1))

  override def toString = if (isLeaf) "_" else
    "(" + children.map(_.toString).mkString(",") + ")"
}