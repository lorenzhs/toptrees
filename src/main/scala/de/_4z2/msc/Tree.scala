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

class Node[DataType](var parent: Node[DataType]) {
  var data: Option[DataType] = None
  var index:Int = -1
  var mergeType:Int = -1
  var children = new ArrayBuffer[Node[DataType]]()
  def this(_data: DataType) { this(null); data = Some(_data) }
  def this(parent: Node[DataType], _data: DataType) { this(parent); data = Some(_data) }

  def addChild(child: Node[DataType]):Node[DataType] = { child.index = numChildren; children += child; child }
  def newChild(data: DataType):Node[DataType] = addChild(new Node[DataType](this, data))
  def newChild():Node[DataType] = addChild(new Node[DataType](this))
  def setChild(index: Int, child: Node[DataType]) = { child.index = index; children(index) = child; child }
  def removeChild(index: Int) { children.remove(index); children.foreach(child => if (child.index > index) child.index -= 1) }
  def isLeaf = children.isEmpty
  def isInnerNode = parent != null && !children.isEmpty
  def isRoot = parent == null;
  def numChildren = children.size
  def hasSiblings = parent match {
    case null => false
    case node => node.children.size > 1
  }
  def isFirstChild = { assert(!isRoot); index == 0 }
  def isLastChild = { assert(!isRoot || { println(toString()); false }); index == parent.numChildren-1 }
  def leftSibling = if (isFirstChild) None else Some(parent.children(index - 1))
  def rightSibling = if (isLastChild) None else Some(parent.children(index + 1))

  def mergeWithLeftSibling = { assert(!isFirstChild); mergeWithSibling(index-1) }
  def mergeWithRightSibling = { assert(!isLastChild); mergeWithSibling(index+1) }
  private def mergeWithSibling(otherIndex: Int) = {
    assert(Math.abs(index-otherIndex) == 1)
    val left  = parent.children(Math.min(index, otherIndex))
    var right = parent.children(Math.max(index, otherIndex))

    val newNode = new Node[DataType](parent)
    newNode.mergeType = (left.isLeaf, right.isLeaf) match {
      case (true, true)  => 4
      case (true, false) => { newNode.children = right.children; 3 }
      case (false, true) => { newNode.children = left.children; 2 }
      case _ => { assert(false); -1 }  // not allowed
    }
    parent.setChild(Math.min(index, otherIndex), newNode)
    parent.removeChild(Math.max(index, otherIndex))
    newNode  // return new node
  }

  def mergeWithOnlyChild = {
    assert(numChildren == 1)
    val child = children(0)
    if (child.isLeaf) {
      mergeType = 1
      children.clear()
    } else {
      child.mergeType = 0
      child.parent = parent
      parent.setChild(index, child);
    }
  }

  override def toString = if (isLeaf) data.getOrElse("_").toString else
    "(" + data.getOrElse("_").toString + "/" + children.map(_.toString).mkString(",") + ")"
}
