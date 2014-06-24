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

  def foreach[U](f: (BinNode[DataType]) => U): Unit = {
    // Pre-order node traversal
    left.foreach(f(_))
    right.foreach(f(_))
    f(this)
  }

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
  def isLastChild = { assert(!isRoot); index == parent.numChildren-1 }
  def leftSibling = if (isFirstChild) None else Some(parent.children(index - 1))
  def rightSibling = if (isLastChild) None else Some(parent.children(index + 1))

  def foreach[U](f: (Node[DataType]) => U): Unit = {
    // Pre-order traversal
    children.foreach(_.foreach(f))
    f(this)
  }

  def mergeWithLeftSibling(data: Option[DataType] = None): Node[DataType] = { assert(!isFirstChild); mergeWithSibling(index-1, data) }
  def mergeWithRightSibling(data: Option[DataType] = None): Node[DataType] = { assert(!isLastChild); mergeWithSibling(index+1, data) }
  def mergeWithLeftSibling(data: DataType): Node[DataType] = mergeWithLeftSibling(Some(data))
  def mergeWithRightSibling(data: DataType): Node[DataType] = mergeWithRightSibling(Some(data))
  private def mergeWithSibling(otherIndex: Int, data: Option[DataType]) = {
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
    newNode.data = data
    parent.setChild(Math.min(index, otherIndex), newNode)
    parent.removeChild(Math.max(index, otherIndex))
    newNode  // return new node
  }

  def mergeWithOnlyChild(data: DataType): Node[DataType] = mergeWithOnlyChild(Some(data))
  def mergeWithOnlyChild(_data: Option[DataType] = None): Node[DataType] = {
    assert(numChildren == 1)
    val child = children(0)
    if (child.isLeaf) {
      mergeType = 1
      data = _data
      children.clear()
      this
    } else {
      child.mergeType = 0
      child.parent = parent
      child.data = _data
      parent.setChild(index, child);
      child
    }
  }

  override def toString = if (isLeaf) data.getOrElse("_").toString else
    "(" + data.getOrElse("_").toString + "/" + children.map(_.toString).mkString(",") + ")"
}

trait NodeInt {
  var firstEdgeIndex: Int
  var lastEdgeIndex: Int
  var parent: Int
  def numEdges:Int = lastEdgeIndex - firstEdgeIndex + 1
  def isLeaf = numEdges == 0
}

trait EdgeInt[T <: EdgeInt[T]] {
  var valid: Boolean
  var headNode: Int
  def copy: T
}

class TreeNode extends NodeInt {
  var firstEdgeIndex = -1
  var lastEdgeIndex = -1
  var parent = -1

  override def toString = "(" + parent + ";" + firstEdgeIndex + "→" + lastEdgeIndex + ")"
}

class TreeEdge extends EdgeInt[TreeEdge] {
  var valid = false
  var headNode = -1
  def this(v: Boolean, h:Int) { this(); valid=v; headNode=h;}
  def copy = new TreeEdge(valid, headNode)

  override def toString = "(" + headNode + "/" + valid.toString.charAt(0) + ")"
}

// Adjacency array data structure that is mostly a general graph data structure
// (the parent pointers obviously break this, and the cluster merging as well)
// I implemented this as a graph in C++ a while ago so I remember some of the tricks
// Especially addEdge takes a while to get right
class OrderedTree[NodeType <: NodeInt, EdgeType <: EdgeInt[EdgeType]](val nodeFactory: () => NodeType, val edgeFactory: () => EdgeType) {
  val nodes = new ArrayBuffer[NodeType]();
  val edges = (new ArrayBuffer[EdgeType]() += edgeFactory());  // add a dummy edge
  private var _numEdges: Int = 0
  private var _firstFreeEdge: Int = 1

  def addNode = {
    val node = nodeFactory();
    node.firstEdgeIndex = _firstFreeEdge;
    node.lastEdgeIndex = _firstFreeEdge - 1;
    nodes += node;
    nodes.size - 1
  }
  def addNodes(n: Int) = { (1 to n).foreach(i => addNode); nodes.size - n }
  def removeNode(node: Int) { outgoingEdges(node).foreach(_.valid = false); nodes(node).lastEdgeIndex = nodes(node).firstEdgeIndex - 1 }

  private def prepareEdge(index: Int, tail:Int, head: Int): EdgeType = {
    nodes(head).parent = tail
    _numEdges += 1
    edges(index) = edgeFactory()
    edges(index).valid = true
    edges(index).headNode = head
    edges(index)
  }

  def addEdge(from: Int, to: Int): EdgeType = {
    val source = nodes(from)
    // Check for space to the right
    var newId = source.lastEdgeIndex + 1
    if (newId < edges.size && !edges(newId).valid) {
      source.lastEdgeIndex += 1
      if (newId == _firstFreeEdge) _firstFreeEdge += 1
      return prepareEdge(newId, from, to)
    }

    // Check for space to the left
    newId = source.firstEdgeIndex - 1;
    if (newId > 0 && !edges(newId).valid) {  // 0 is the dummy edge
      source.firstEdgeIndex -= 1
      return prepareEdge(newId, from, to)
    }

    // Move edges of source vertex to the end
    if (source.numEdges >= edges.size - _firstFreeEdge)
      (edges.size to _firstFreeEdge + source.numEdges).foreach(i => edges += edgeFactory())
    if (source.lastEdgeIndex == _firstFreeEdge - 1)
      newId = _firstFreeEdge // if we happen to be at the end already,we can just append
    else {
      // otherwise, move source node's edges to the end
      newId = _firstFreeEdge + source.numEdges
      (0 until source.numEdges).foreach(i => {
        edges(_firstFreeEdge + i) = edges(source.firstEdgeIndex + i).copy
        edges(source.firstEdgeIndex + i).valid = false
      })
      source.firstEdgeIndex = _firstFreeEdge
    }
    source.lastEdgeIndex = newId
    _firstFreeEdge = newId + 1
    return prepareEdge(newId,from, to)
  }

  // XXX this may be hard to use because it requires the edge ID
  def removeEdge(from: Int, edge: Int) { removeEdge(nodes(from), edge) }
  def removeEdge(from: NodeType, edgeId: Int, compact: Boolean = true) {
    assert(edges(edgeId).valid)
    val last = from.lastEdgeIndex
    if (edgeId == last)
      edges(edgeId).valid = false
    else {
      // edge is somewhere in the middle
      // edges need to remain ordered -> copy all of them :(
      if (compact) {
        (edgeId + 1 to last).foreach(index => {
          edges(index-1) = edges(index).copy
        })
        edges(last).valid = false
      } else
        edges(edgeId).valid = false
    }
    from.lastEdgeIndex -= 1
    _numEdges -= 1
  }
  def removeEdgeTo(from: Int, to: Int) { removeEdgeTo(nodes(from), to) }
  def removeEdgeTo(from: NodeType, to: Int) { removeEdge(from, (from.firstEdgeIndex to from.lastEdgeIndex).toIterator.filter(e => e == to).next()) }

  // returns (merged node id, merge type)
  def mergeNodeWithOnlyChild(nodeId: Int): (Int,Int) = {
    assert(nodes(nodeId).numEdges == 1)
    val childId = childrenIds(nodeId).next()
    val child = nodes(childId)
    removeEdgeTo(nodeId, childId) // remove edge from node to to child
    if (child.numEdges == 0) {
      return (nodeId, 1)
    } else {
      child.parent = nodes(nodeId).parent
      addEdge(child.parent, childId)
      return (childId, 0)
    }
  }

  // returns (merged node id, merge type)
  def mergeNodeWithSibling(nodeId: Int, siblingId: Int):(Int,Int) = {
    val parent = nodes(nodeId).parent
    assert(parent == nodes(siblingId).parent)
    val edgeToNode = getEdgeId(parent, nodeId)
    val edgeToSibling = getEdgeId(parent, siblingId)

    assert(nodes(nodeId).isLeaf || nodes(siblingId).isLeaf)

    val leftId  = Math.min(edgeToNode, edgeToSibling)
    val rightId = Math.max(edgeToNode, edgeToSibling)
    val left = nodes(leftId)
    val right = nodes(rightId)
    val mergeType = (left.isLeaf, right.isLeaf) match {
      case (true, true)  => 4
      case (true, false) => 3
      case (false, true) => 2
      case _ => -1
    }
    if (right.isLeaf) {
      // left is the new vertex
      removeEdgeTo(parent, rightId);
      return (leftId, mergeType);
    } else {
      // right is the new vertex
      removeEdgeTo(parent, leftId);
      return (rightId, mergeType);
    }
  }

  // Read-only Getter
  def numEdges = _numEdges
  def numNodes = nodes.size

  def firstEdge(node: Int) = edges(nodes(node).firstEdgeIndex)
  def lastEdge(node: Int)  = edges(nodes(node).lastEdgeIndex)

  def hasEdge(from: Int, to: Int): Boolean = { outgoingEdges(from).foreach(e => if (e.headNode == to) return true); false }
  def getEdge(from: Int, to: Int): Option[EdgeType] = { outgoingEdges(from).foreach(e => if (e.headNode == to) return Some(e)); None }
  def getEdgeId(from: Int, to:Int): Int = { outgoingEdgeIds(from).foreach(e => if (edges(e).headNode == to) return e); -1 }

  def outgoingEdges(node: Int):Iterator[EdgeType] = outgoingEdges(nodes(node))
  def outgoingEdgeIds(node: Int):Iterator[Int] = outgoingEdgeIds(nodes(node))
  def outgoingEdges(node: NodeType):Iterator[EdgeType] = outgoingEdgeIds(node).map(edges(_))
  def outgoingEdgeIds(node: NodeType):Iterator[Int] = (node.firstEdgeIndex to node.lastEdgeIndex).toIterator

  def children(node: Int):Iterator[NodeType] = children(nodes(node))
  def childrenIds(node: Int):Iterator[Int] = childrenIds(nodes(node))
  def children(node: NodeType):Iterator[NodeType] = childrenIds(node).map(nodes(_))
  def childrenIds(node: NodeType):Iterator[Int] = (node.firstEdgeIndex to node.lastEdgeIndex).toIterator.map(edges(_).headNode)

  def siblings(node:Int):Iterator[NodeType] = siblingIds(node).map(nodes(_))
  def siblingIds(node:Int):Iterator[Int] = nodes(node).parent match {
    case -1 => Iterator.empty
    case parent => childrenIds(parent).filter(c => c != node)  // id (Int) comparison, fast
  }
  def siblings(node:NodeType):Iterator[NodeType]  = node.parent match {
    case -1 => Iterator.empty
    case parent => children(parent).filter(c => c != node)  // object comparison, slow
  }
  def siblingIds(node:NodeType):Iterator[Int] = node.parent match {
    case -1 => Iterator.empty
    case parent => childrenIds(parent).filter(c => nodes(c) != node)  // object comparison and stupid wrapping, extra slow
  }

  override def toString = "Ordered tree with " + numNodes + " nodes and " + numEdges + " edges\nnodes: " + nodes + "\nedges: " + edges
}
