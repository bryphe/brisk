module Create = (Node: Flex.Spec.Node, Encoding: Flex.Spec.Encoding) => {
  include Flex.Layout.Create(Node, Encoding);
  include Style.Create(Encoding);

  module Color = Color0;

  open Encoding;
  open LayoutSupport;
  open LayoutTypes;

  let flexDirection = d =>
    `flexDirection(
      switch (d) {
      | `column => Column
      | `columnReverse => ColumnReverse
      | `row => Row
      | `rowReverse => RowReverse
      },
    );

  let convertAlign =
    fun
    | `auto => AlignAuto
    | `flexStart => AlignFlexStart
    | `center => AlignCenter
    | `flexEnd => AlignFlexEnd
    | `stretch => AlignStretch;

  let justifyContent = j =>
    `justifyContent(
      switch (j) {
      | `flexStart => JustifyFlexStart
      | `center => JustifyCenter
      | `flexEnd => JustifyFlexEnd
      | `spaceBetween => JustifySpaceBetween
      | `spaceAround => JustifySpaceAround
      },
    );

  let alignContent = a => `alignContent(convertAlign(a));
  let alignItems = a => `alignItems(convertAlign(a));
  let alignSelf = a => `alignSelf(convertAlign(a));

  type style = [
    | `position(Position.t)
    | `flexDirection(flexDirection)
    | `justifyContent(justify)
    | `alignContent(align)
    | `alignItems(align)
    | `alignSelf(align)
    | `width(scalar)
    | `border(Border.t)
    | `height(scalar)
    | `padding(scalar, scalar, scalar, scalar)
    | `margin(scalar, scalar, scalar, scalar)
  ];

  let isUndefined = Encoding.isUndefined;

  let int_of_scalar = s => s |> Encoding.scalarToFloat |> int_of_float;

  let applyCommonStyle = (style, attr: [> style]) =>
    switch (attr) {
    | `position(({position, left, top, right, bottom}: Position.t)) =>
      let positionType =
        switch (position) {
        | `absolute => Absolute
        | `relative => Relative
        };

      let style = {...style, positionType};

      let style =
        !isUndefined(left) ? {...style, left: int_of_scalar(left)} : style;
      let style =
        !isUndefined(top) ? {...style, top: int_of_scalar(top)} : style;
      let style =
        !isUndefined(right) ?
          {...style, right: int_of_scalar(right)} : style;
      let style =
        !isUndefined(bottom) ?
          {...style, bottom: int_of_scalar(bottom)} : style;
      style;
    | `flexDirection(flexDirection) => {...style, flexDirection}
    | `justifyContent(justifyContent) => {...style, justifyContent}
    | `alignContent(alignContent) => {...style, alignContent}
    | `alignItems(alignItems) => {...style, alignItems}
    | `alignSelf(alignSelf) => {...style, alignSelf}
    | `width(w) => {...style, width: int_of_scalar(w)}
    | `height(h) => {...style, height: int_of_scalar(h)}
    | `border(({width, _}: Border.t)) =>
      !isUndefined(width) ? {...style, border: int_of_scalar(width)} : style
    | `padding(l, t, r, b) =>
      let style =
        !isUndefined(l) ? {...style, paddingLeft: int_of_scalar(l)} : style;
      let style =
        !isUndefined(t) ? {...style, paddingTop: int_of_scalar(t)} : style;
      let style =
        !isUndefined(r) ?
          {...style, paddingRight: int_of_scalar(r)} : style;
      let style =
        !isUndefined(b) ?
          {...style, paddingBottom: int_of_scalar(b)} : style;
      style;
    | `margin(l, t, r, b) =>
      let style =
        !isUndefined(l) ? {...style, marginLeft: int_of_scalar(l)} : style;
      let style =
        !isUndefined(t) ? {...style, marginTop: int_of_scalar(t)} : style;
      let style =
        !isUndefined(r) ? {...style, marginRight: int_of_scalar(r)} : style;
      let style =
        !isUndefined(b) ?
          {...style, marginBottom: int_of_scalar(b)} : style;
      style;
    | _ => style
    };

  let makeLayoutNode = (~measure=?, ~style, hostView: Node.context) => {
    let accum = {
      ...LayoutSupport.defaultStyle,
      direction: LayoutSupport.defaultStyle.direction,
    };

    let andStyle = List.fold_left(applyCommonStyle, accum, style);
    LayoutSupport.createNode(
      ~withChildren=[||],
      ~andStyle,
      ~andMeasure=?measure,
      hostView,
    );
  };

  let cssNodeInsertChild = (node, child, index) => {
    assert(child.parent === theNullNode);
    assert(node.measure === None);
    let capacity = Array.length(node.children);
    if (capacity == node.childrenCount) {
      /* TODO:Simply use Array.fill (no need to allocate a separate `fill` array
       * */
      let fill = Array.make(capacity + 1, theNullNode);
      Array.blit(node.children, 0, fill, 0, capacity);
      node.children = fill;
    };
    for (i in node.childrenCount downto index + 1) {
      node.children[i] = node.children[i - 1];
    };
    node.childrenCount = node.childrenCount + 1;

    node.children[index] = child;
    child.parent = node;

    markDirtyInternal(node);
  };
};