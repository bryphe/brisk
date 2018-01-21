open React_ios;

module ReasonReact = {
  module Implementation = {
    let map = Hashtbl.create(1000);
    type hostView =
      | Text(string)
      | View;
    let getInstance = (id) =>
      if (Hashtbl.mem(map, id)) {
        Some(Hashtbl.find(map, id))
      } else {
        None
      };
  };
  include ReactCore_Internal.Make(Implementation);
  module Text = {
    let component = statelessNativeComponent("Text");
    let make = (~title="ImABox", ~onClick as _=?, _children) => {
      ...component,
      render: (_) => {
        setProps: (_) => (),
        children: listToElement([]),
        style: Layout.defaultStyle,
        make: (id) => {
          let elem = Implementation.Text(title);
          Hashtbl.add(Implementation.map, id, elem);
          elem
        }
      }
    };
    let createElement = (~key=?, ~title=?, ~children as _children, ()) =>
      element(~key?, make(~title?, ()));
  };
  let stringToElement = (string) => <Text title=string />;
};

module TestRenderer = {
  open ReasonReact;
  type opaqueComponent =
    | Component(component('a, 'b, 'c)): opaqueComponent
    | InstanceAndComponent(component('a, 'b, 'c), instance('a, 'b, 'c)): opaqueComponent;
  let compareComponents = false;
  type testInstance = {
    component: opaqueComponent,
    id: Key.t,
    subtree: t
  }
  and t = list(testInstance);
  let rec convertInstance =
    fun
    | Instance({component, id, instanceSubTree} as instance) => {
        component: InstanceAndComponent(component, instance),
        id,
        subtree: convertElement(instanceSubTree)
      }
    | NativeInstance(_, {component, id, instanceSubTree} as instance) => {
        component: InstanceAndComponent(component, instance),
        id,
        subtree: convertElement(instanceSubTree)
      }
  and convertElement =
    fun
    | IFlat(instances) => List.map(convertInstance, instances)
    | INested(_, elements) => List.flatten(List.map(convertElement, elements));
  let render = (element) => convertElement(RenderedElement.render(element));
  let update = (element, next) => {
    let (element, _) = RenderedElement.update(element, next);
    convertElement(element)
  };
  let compareComponents = (left, right) =>
    switch (left, right) {
    | (Component(_), Component(_))
    | (InstanceAndComponent(_, _), InstanceAndComponent(_, _)) => assert false
    | (Component(justComponent), InstanceAndComponent(comp, instance)) =>
      comp.handedOffInstance := Some(instance);
      let result =
        switch justComponent.handedOffInstance^ {
        | Some(_) => true
        | None => false
        };
      comp.handedOffInstance := None;
      result
    | (InstanceAndComponent(comp, instance), Component(justComponent)) =>
      comp.handedOffInstance := Some(instance);
      let result =
        switch justComponent.handedOffInstance^ {
        | Some(_) => true
        | None => false
        };
      comp.handedOffInstance := None;
      result
    };
  let rec compareElement = (left, right) =>
    switch (left, right) {
    | (le, re) =>
      if (List.length(le) != List.length(re)) {
        false
      } else {
        List.fold_left(
          (&&),
          true,
          List.map(compareInstance, List.combine(le, re))
        )
      }
    }
  and compareInstance = ((left, right)) =>
    left.id == right.id
    && compareComponents(left.component, right.component)
    && compareElement(left.subtree, right.subtree);
  let printList = (indent, lst) => {
    let indent = String.make(indent, ' ');
    "["
    ++ String.concat(",\n", List.map((s) => s, lst))
    ++ "\n"
    ++ indent
    ++ "]"
  };
  let componentName = (component) =>
    switch component {
    | InstanceAndComponent(component, _) => component.debugName
    | Component(component) => component.debugName
    };
  let printElement = (tree) => {
    let rec pp = (~indent, instances) =>
      ListTR.map(printInstance(~indent), instances) |> printList(indent)
    and printInstance = (~indent, instance) => {
      let indentString = String.make(indent, ' ');
      Printf.sprintf(
        {|
      %s{
      %s  id: %s
      %s  name: %s
      %s  subtree: %s
      %s}
      |},
        indentString,
        indentString,
        string_of_int(instance.id),
        indentString,
        componentName(instance.component),
        indentString,
        pp(~indent=indent + 2, instance.subtree),
        indentString
      )
    };
    pp(~indent=0, tree)
  };
};


/***
 * The simplest component. Composes nothing!
 */
module Box = {
  open ReasonReact;
  let component = statelessNativeComponent("Box");
  let make = (~title="ImABox", ~onClick as _=?, _children) => {
    ...component,
    render: (_) => {
      setProps: (_) => (),
      children: ReasonReact.listToElement([]),
      style: Layout.defaultStyle,
      make: (id) => {
        let elem = Implementation.Text(title);
        Hashtbl.add(Implementation.map, id, elem);
        elem
      }
    }
  };
  let createElement = (~key=?, ~title=?, ~children as _children, ()) =>
    ReasonReact.element(~key?, make(~title?, ()));
};

module Div = {
  open ReasonReact;
  let component = statelessNativeComponent("Div");
  let make = (children) => {
    ...component,
    render: (_) => {
      setProps: (_) => (),
      children: listToElement(children),
      style: Layout.defaultStyle,
      make: (id) => {
        let elem = Implementation.View;
        Hashtbl.add(Implementation.map, id, elem);
        elem
      }
    }
  };
  let createElement = (~key=?, ~children, ()) =>
    ReasonReact.element(~key?, make(children));
};

module BoxWrapper = {
  let component = ReasonReact.statelessComponent("BoxWrapper");
  let make =
      (~title="ImABox", ~twoBoxes=false, ~onClick as _=?, _children)
      : ReasonReact.component(
          ReasonReact.stateless,
          unit,
          ReasonReact.reactElement
        ) => {
    ...component,
    initialState: () => (),
    render: (_self) =>
      twoBoxes ?
        <Div> <Box title /> <Box title /> </Div> : <Div> <Box title /> </Div>
  };
  let createElement = (~key=?, ~title=?, ~twoBoxes=?, ~children, ()) =>
    ReasonReact.element(~key?, make(~title?, ~twoBoxes?, ~onClick=(), ()));
};


/***
 * Box with dynamic keys.
 */
module BoxWithDynamicKeys = {
  let component =
    ReasonReact.statelessComponent(~useDynamicKey=true, "BoxWithDynamicKeys");
  let make = (~title="ImABox", _children: list(ReasonReact.reactElement)) => {
    ...component,
    render: (_self) => <Box title />
  };
  let createElement = (~title, ~children, ()) =>
    ReasonReact.element(make(~title, children));
};

module BoxList = {
  type action =
    | Create(string)
    | Reverse;
  let component = ReasonReact.reducerComponent("BoxList");
  let make = (~rAction, ~useDynamicKeys=false, _children) => {
    ...component,
    initialState: () => [],
    reducer: (action, state) =>
      switch action {
      | Create(title) =>
        ReasonReact.Update([
          useDynamicKeys ? <BoxWithDynamicKeys title /> : <Box title />,
          ...state
        ])
      | Reverse => ReasonReact.Update(List.rev(state))
      },
    render: ({state, act}) => {
      ReasonReact.RemoteAction.subscribe(~act, rAction);
      ReasonReact.listToElement(state)
    }
  };
  let createElement = (~rAction, ~children, ()) =>
    ReasonReact.element(make(~rAction, children));
};


/***
 * This component demonstrates several things:
 *
 * 1. Demonstration of making internal state hidden / abstract. Components
 * should encapsulate their state representation and should be free to change
 * it.
 *
 * 2. Demonstrates an equivalent of `componentWillReceiveProps`.
 * `componentWillReceiveProps` is like an "edge trigger" on props, and the
 * first item of the tuple shows how we implement that with this API.
 */
module ChangeCounter = {
  type state = {
    numChanges: int,
    mostRecentLabel: string
  };
  let component = ReasonReact.reducerComponent("ChangeCounter");
  let make = (~label, _children) => {
    ...component,
    initialState: () => {mostRecentLabel: label, numChanges: 10},
    reducer: ((), state) =>
      ReasonReact.Update({...state, numChanges: state.numChanges + 1000}),
    willReceiveProps: ({state, reduce}) =>
      label != state.mostRecentLabel ?
        {
          reduce(() => (), ());
          reduce(() => (), ());
          {mostRecentLabel: label, numChanges: state.numChanges + 1}
        } :
        state,
    render: ({state: {numChanges, mostRecentLabel}}) => {
      let title = Printf.sprintf("[%d, \"%s\"]", numChanges, mostRecentLabel);
      <Div> <Box title /> </Div>
    }
  };
  let createElement = (~label, ~children, ()) =>
    ReasonReact.element(make(~label, ()));
};

module StatelessButton = {
  let component = ReasonReact.statelessComponent("StatelessButton");
  let make =
      (~initialClickCount as _="noclicks", ~test as _="default", _children) => {
    ...component,
    render: (_self) => <Div> <Box /> </Div>
  };
  let createElement = (~initialClickCount=?, ~test=?, ~children, ()) =>
    ReasonReact.element(make(~initialClickCount?, ~test?, ()));
};

module ButtonWrapper = {
  type state = {buttonWrapperState: int};
  let component = ReasonReact.statefulComponent("ButtonWrapper");
  let make = (~wrappedText="default", _children) => {
    ...component,
    initialState: () => {buttonWrapperState: 0},
    render: ({state}) =>
      <Div>
        (ReasonReact.stringToElement(string_of_int(state.buttonWrapperState)))
        <StatelessButton
          initialClickCount=("wrapped:" ++ (wrappedText ++ ":wrapped"))
        />
        <StatelessButton
          initialClickCount=("wrapped:" ++ (wrappedText ++ ":wrapped"))
        />
      </Div>
  };
  let createElement = (~wrappedText=?, ~children, ()) =>
    ReasonReact.element(make(~wrappedText?, ()));
};

let buttonWrapperJsx = <ButtonWrapper wrappedText="TestButtonUpdated!!!" />;

module ButtonWrapperWrapper = {
  let component = ReasonReact.statefulComponent("ButtonWrapperWrapper");
  let make = (~wrappedText="default", _children) => {
    ...component,
    initialState: () => "buttonWrapperWrapperState",
    render: ({state}) =>
      <Div>
        (ReasonReact.stringToElement(state))
        (ReasonReact.stringToElement("wrappedText:" ++ wrappedText))
        buttonWrapperJsx
      </Div>
  };
};

module UpdateAlternateClicks = {
  type action =
    | Click;
  let component = ReasonReact.reducerComponent("UpdateAlternateClicks");
  let make = (~rAction, _children) => {
    ...component,
    initialState: () => 0,
    printState: (state) => string_of_int(state),
    reducer: (Click, state) => Update(state + 1),
    shouldUpdate: ({newSelf: {state}}) => state mod 2 === 0,
    render: ({state, act}) => {
      ReasonReact.RemoteAction.subscribe(~act, rAction);
      ReasonReact.stringToElement("Rendered state is " ++ string_of_int(state))
    }
  };
};

let renderedElement =
  Alcotest.testable(
    (formatter, t) =>
      Format.pp_print_text(formatter, TestRenderer.printElement(t)),
    TestRenderer.compareElement
  );

let suite = [
  (
    "First Render",
    `Quick,
    () => {
      open TestRenderer;
      let component = BoxWrapper.make();
      let rendered = render(ReasonReact.element(component));
      let expected = [
        {
          component: Component(BoxWrapper.component),
          id: 0,
          subtree: [
            {
              component: Component(Div.component),
              id: 1,
              subtree: [
                {component: Component(Box.component), id: 2, subtree: []}
              ]
            }
          ]
        }
      ];
      Alcotest.check(renderedElement, "", expected, rendered)
    }
  ),
  (
    "Top level update",
    `Quick,
    () => {
      open TestRenderer;
      let component = BoxWrapper.make();
      let rendered =
        update(
          ReasonReact.RenderedElement.render(<BoxWrapper />),
          <BoxWrapper twoBoxes=true />
        );
      let expected = [
        {
          component: Component(component),
          id: 0,
          subtree: [{component: Component(Div.make([])), id: 1, subtree: []}]
        }
      ];
      Alcotest.check(renderedElement, "", expected, rendered)
    }
  )
];

Alcotest.run("Tests", [("BoxWrapper", suite)]);
