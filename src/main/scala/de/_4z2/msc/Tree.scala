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
  var lastMergedIn: Int
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
  var lastMergedIn = -1

  override def toString = "(" + parent + ";" + firstEdgeIndex + "→" + lastEdgeIndex + ";" + lastMergedIn + ")"
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

  // prepares edge from `tail` to `head``and stores it with ID `index`
  private def prepareEdge(index: Int, tail:Int, head: Int): EdgeType = {
    nodes(head).parent = tail
    _numEdges += 1
    edges(index) = edgeFactory()
    edges(index).valid = true
    edges(index).headNode = head
    edges(index)
  }

  // insert an edge (`from`, `to`)
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
      _copyEdges(newId + 1, newId, source.numEdges)
      return prepareEdge(newId + source.numEdges + 1, from, to)
    }

    // Move edges of source vertex to the end
    if (source.numEdges >= edges.size - _firstFreeEdge)
      (edges.size to _firstFreeEdge + source.numEdges).foreach(i => edges += edgeFactory())
    if (source.lastEdgeIndex == _firstFreeEdge - 1)
      newId = _firstFreeEdge // if we happen to be at the end already,we can just append
    else {
      // otherwise, move source node's edges to the end
      newId = _firstFreeEdge + source.numEdges
      _moveEdges(source.firstEdgeIndex, _firstFreeEdge, source.numEdges)
      source.firstEdgeIndex = _firstFreeEdge
    }
    source.lastEdgeIndex = newId
    _firstFreeEdge = newId + 1
    return prepareEdge(newId, from, to)
  }

  // copy `num` edges around (left-to-right)
  private def _copyEdges(origIndex: Int, newIndex: Int, num: Int): Unit =
    (0 until num).foreach(i => edges(newIndex + i) = edges(origIndex + i).copy)
  // move `num` edges, invalidating the old ones
  private def _moveEdges(origIndex: Int, newIndex: Int, num: Int): Unit =
    (0 until num).foreach(i => {
      edges(newIndex + i) = edges(origIndex + i).copy
      edges(origIndex + i).valid = false
    })

  // Helper functions for edge insertion
  private def _addEdgeLeft(from: Int, to: Int, index: Int) : Option[EdgeType] = {
    val source = nodes(from)
    if (!edges(source.firstEdgeIndex-1).valid) {
      return None
    }
    _copyEdges(source.firstEdgeIndex - 1, source.firstEdgeIndex, index)
    source.firstEdgeIndex -= 1
    return Some(prepareEdge(source.firstEdgeIndex + index, from, to))
  }
  private def _addEdgeRight(from: Int, to: Int, index: Int): Option[EdgeType] = {
    val source = nodes(from)
    if(!edges(source.lastEdgeIndex+1).valid) {
      return None
    }
    (source.numEdges -1 to index by -1).foreach(i => {
      edges(source.firstEdgeIndex + i + 1) = edges(source.firstEdgeIndex + i).copy
    })
    source.lastEdgeIndex += 1
    return Some(prepareEdge(source.firstEdgeIndex + index, from, to))
  }

  // insert an edge (`from`, `to`) in a specific position of `from`'s outgoing edges
  def addEdge(from: Int, to: Int, index: Int): EdgeType = {
    val source = nodes(from)
    assert(source.numEdges >= index)
    if (source.numEdges == index) return addEdge(from, to)  // simple case

    if (2*index < source.numEdges) {
      // try moving the first part left first
      _addEdgeLeft(from, to, index) match {
        case Some(edge) => return edge
        case None => {}  // too bad
      }
      _addEdgeRight(from, to, index) match {
        case Some(edge) => return edge
        case None => {} // couldn't insert on the right side either
      }
    } else {
       // try moving the last part right first
      _addEdgeRight(from, to, index) match {
        case Some(edge) => return edge
        case None => {}  // no space on the right
      }
      _addEdgeLeft(from, to, index) match {
        case Some(edge) => return edge
        case None => {} // nor is there space on the left
      }
    }

    // we have to move the node's edges to the end
    if (source.numEdges >= edges.size - _firstFreeEdge)
      (edges.size to _firstFreeEdge + source.numEdges).foreach(i => edges += edgeFactory())
    if (source.lastEdgeIndex == _firstFreeEdge - 1) {
      // vertex is at the end already
      return _addEdgeRight(from, to, index).get
    } else {
      // move to the end, leaving a gap at index into which we then insert the new edge
      _moveEdges(source.firstEdgeIndex, _firstFreeEdge, index)
      _moveEdges(source.firstEdgeIndex + index, _firstFreeEdge + index + 1, (source.numEdges - index))

      val numChildren = source.numEdges + 1
      source.firstEdgeIndex = _firstFreeEdge
      source.lastEdgeIndex = _firstFreeEdge + numChildren
      _firstFreeEdge += numChildren
      return prepareEdge(_firstFreeEdge + index, from, to)
    }
  }

  // remove outgoing edge of from by (global) edge ID
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
        _copyEdges(edgeId + 1, edgeId, last-edgeId)
        edges(last).valid = false
      } else
        edges(edgeId).valid = false
    }
    from.lastEdgeIndex -= 1
    _numEdges -= 1
  }
  // remove the first edge from node `from` to node `to`
  def removeEdgeTo(from: Int, to: Int) { /*println("removing edge from " + from + " to " + to);*/ removeEdgeTo(nodes(from), to) }
  def removeEdgeTo(from: NodeType, to: Int) {
    removeEdge(from, (from.firstEdgeIndex to from.lastEdgeIndex).toIterator.filter(e => edges(e).headNode == to).next())
  }

  // do merges until only one edge is left
  // the callback is called for every pair of merged nodes, its signature:
  // def callback(node1: Int, node2: Int, newNode: Int, mergeType:Int): Unit
  def doMerges(mergeCallback: (Int, Int, Int, Int) => Unit) {
    var iteration = 0
    while (_numEdges > 1) {
      println("Horizontal merges, iteration " + iteration)
      horizontalMerges(iteration, mergeCallback)
      println("Vertical merges merges, iteration " + iteration)
      verticalMerges(iteration, mergeCallback)
      println(this)
      println()
      iteration += 1
    }
  }

  def horizontalMerges(iteration: Int, callback: (Int, Int, Int, Int) => Unit) = {
    nodes.filter(_.numEdges >= 2).foreach(node => {
      var (first, last) = childrenIds(node).grouped(2).partition(_.size == 2)
      first.toList.filter(_.exists(nodes(_).isLeaf)).foreach(pair => {
          val n1 = pair(0)
          val n2 = pair(1)
          pair.foreach(n => nodes(n).lastMergedIn = iteration)
          //print("\tmerging nodes " + n1 + " and " + n2 + ", common parent " + nodes(n1).parent + "; ")
          val (newNode, mergeType) = mergeNodeWithSibling(n1, n2)
          callback(n1, n2, newNode, mergeType)
          //println("new node: " + newNode)
      })
      last.toList.foreach(list => {
        assert(list.size == 1)
        val node = list(0)
        val parent = nodes(nodes(node).parent)
        if (nodes(node).isLeaf && parent.numEdges > 2) {
          val sib = (1 to 2).map(i => edges(parent.lastEdgeIndex - i).headNode)
          if (!sib.exists(nodes(_).isLeaf)) {
            //println("\tmerging odd node " + node + " with its left neighbour " + sib(0))
            nodes(node).lastMergedIn = iteration
            nodes(sib(0)).lastMergedIn = iteration
            val (newNode, mergeType) = mergeNodeWithSibling(node, sib(0))
            callback(node, sib(0), newNode, mergeType)
          }
        }
      })
    })
  }

  // Extend a chain of nodes upward from `idx`, the bottom node of the chain
  private def _extendVerticalMerges(idx: Int, iteration: Int, callback: (Int, Int, Int, Int) => Unit) {
    //println("\textending vertical merges upward from " + idx)
    var index = idx
    var node = nodes(index)
    var parentId = node.parent
    while(parentId >= 0 && nodes(parentId).numEdges == 1 && nodes(parentId).parent >= 0 && nodes(parentId).lastMergedIn < iteration) {
      node.lastMergedIn = iteration
      nodes(parentId).lastMergedIn = iteration
      // print("\t\tmerging parent " + parentId + " (LM: " + nodes(parentId).lastMergedIn + ") with child " + index + " (LM: " + node.lastMergedIn + "); ")
      val (newNode, mergeType) = mergeNodeWithOnlyChild(parentId)
      callback(parentId, index, newNode, mergeType)
      //println("result node: " + newNode + " merge type: " + mergeType)
      // Go upwards twice to extend the chain, if possible
      index = nodes(newNode).parent
      if (index >= 0)
        node = nodes(index)
      else return
      parentId = node.parent
    }
    //println("\t\taborted vertical merges: parent has " + nodes(parentId).numEdges + " children, parent " + nodes(parentId).parent + ", was last merged in iteration " + nodes(parentId).lastMergedIn)
  }

  def verticalMerges(iteration: Int, callback: (Int, Int, Int, Int) => Unit) = {
    nodes.view.zipWithIndex.filter(pair => pair._1.parent >= 0 && nodes(pair._1.parent).numEdges == 1)  // only child of the parent
      .filter(pair => pair._1.numEdges != 1)  // cannot extend chain downwards
      .foreach(pair => _extendVerticalMerges(pair._2, iteration, callback))  // starting point of such a chain
  }

  // merge a node with its only child (types a=0, b=1)
  // returns (merged node id, merge type)
  def mergeNodeWithOnlyChild(nodeId: Int): (Int,Int) = {
    assert(nodes(nodeId).numEdges == 1)
    val childId = childrenIds(nodeId).next()
    val child = nodes(childId)
    removeEdgeTo(nodeId, childId) // remove edge from node to to child
    child.parent = -1 // just to make this absolutely clear
    if (child.numEdges == 0) {
      return (nodeId, 1)
    } else {
      val node = nodes(nodeId)
      node.firstEdgeIndex = child.firstEdgeIndex
      node.lastEdgeIndex = child.lastEdgeIndex
      child.lastEdgeIndex = child.firstEdgeIndex - 1
      children(node).foreach(_.parent = nodeId)  // attach grandchildren to this node
      return (nodeId, 0)
    }
  }

  // merge a node with one of its siblings (types c=2, d=3, e=4)
  // returns (merged node id, merge type)
  def mergeNodeWithSibling(nodeId: Int, siblingId: Int):(Int,Int) = {
    val parent = nodes(nodeId).parent
    assert(parent == nodes(siblingId).parent)
    val edgeToNode = getEdgeId(parent, nodeId)
    val edgeToSibling = getEdgeId(parent, siblingId)

    assert(nodes(nodeId).isLeaf || nodes(siblingId).isLeaf)

    val leftId  = edges(Math.min(edgeToNode, edgeToSibling)).headNode
    val rightId = edges(Math.max(edgeToNode, edgeToSibling)).headNode
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
      right.parent = -1
      return (leftId, mergeType);
    } else {
      // right is the new vertex
      removeEdgeTo(parent, leftId);
      left.parent = -1
      return (rightId, mergeType);
    }
  }

  // Read-only Getter
  def numEdges = _numEdges
  def numNodes = nodes.size

  // retrieve first/last edge by node ID
  def firstEdge(node: Int) = edges(nodes(node).firstEdgeIndex)
  def lastEdge(node: Int)  = edges(nodes(node).lastEdgeIndex)

  // Check existence or retrieve edge by endpoints (valid edges only)
  def hasEdge(from: Int, to: Int): Boolean = { outgoingEdges(from).foreach(e => if (e.headNode == to) return true); false }
  def getEdge(from: Int, to: Int): Option[EdgeType] = { outgoingEdges(from).foreach(e => if (e.headNode == to) return Some(e)); None }
  def getEdgeId(from: Int, to:Int): Int = { outgoingEdgeIds(from).foreach(e => if (edges(e).valid && edges(e).headNode == to) return e); -1 }

  // All *valid* outgoing edges of `node`
  def outgoingEdges(node: Int):Iterator[EdgeType] = outgoingEdges(nodes(node))
  def outgoingEdges(node: NodeType):Iterator[EdgeType] = outgoingEdgeIds(node).map(edges(_)).filter(_.valid)

  // IDs of all of `node`s outgoing edges (including invalid ones)
  def outgoingEdgeIds(node: Int):Iterator[Int] = outgoingEdgeIds(nodes(node))
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
